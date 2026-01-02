#include "app_ctrl.h"
#include "pid.h"
#include "motor.h"
#include "tim.h"
#include <math.h>
#include <stdlib.h>
#include "key.h" // 引入新按键定义

// --- 私有变量 ---
static PID_Config_t pid_pos;
static PID_Config_t pid_spd;

// --- 全局变量 ---
volatile SystemMode_t current_mode = MODE_IDLE;
float target_val_spd = 0.0f;
float target_val_pos = 0.0f;

// 标志位：演示模式自动旋转
uint8_t is_auto_rotating = 0; 

void App_Ctrl_Init(void)
{
    // 初始化 PID (位置环给硬一点，演示效果好)
    PID_Init(&pid_pos, 5.0f, 0.1f, 0.0f, 45.0f);
    PID_Init(&pid_spd, 20.0f, 7.5f, 0.0f, 3599.0f);
}

// 切换模式逻辑
void App_Ctrl_SetMode(SystemMode_t new_mode)
{
    current_mode = new_mode;
    if (current_mode > MODE_POS) current_mode = MODE_IDLE;

    Motor_Set_Force(0);
    is_auto_rotating = 0; // 切模式时关闭自动旋转
    
    // 重置 PID
    App_Ctrl_Init();

    switch (current_mode)
    {
        case MODE_SPEED:
            target_val_spd = 0.0f; // 默认停
            break;

        case MODE_POS:
            {
                // 就近归零
                float current_turns = round(g_MotorState.total_angle / 360.0f);
                target_val_pos = current_turns * 360.0f;
            }
            break;
            
        case MODE_IDLE:
            // 进 IDLE 也要把目标设为当前，防止误动作
            target_val_pos = g_MotorState.total_angle;
            break;
    }
}

// 🔥🔥🔥 核心按键逻辑 🔥🔥🔥
void App_Ctrl_KeyHandler(uint8_t key)
{
    // === 1. 全局通用逻辑 ===
    
    // 长按 MODE 键：一键回原点 (演示复位神技)
    if (key == KEY_MODE_LONG)
    {
        // 强制切到位置模式并归零
        current_mode = MODE_POS;
        is_auto_rotating = 0;
        target_val_pos = 0.0f; // 回绝对零点
        return;
    }
    
    // === 2. 待机模式 (校准专用) ===
    if (current_mode == MODE_IDLE)
    {
        // UP/DOWN: 电动点动 (帮你在不拧电机的情况下找零点)
        // 注意：这里只给很小的力，松手即停在 Control Loop 里处理
        //if (key == KEY_UP_SHORT || key == KEY_UP_LONG)   Motor_Set_Force(1500); 
        //if (key == KEY_DOWN_SHORT || key == KEY_DOWN_LONG) Motor_Set_Force(-1500);
        
        // 🔥 长按 DOWN：软件设置零点 🔥
        // 对准刻度后，长按 DOWN，告诉单片机“这里就是 0！”
        if (key == KEY_DOWN_LONG)
        {
            g_MotorState.total_angle = 0.0f; // 强行清零
           
            target_val_pos = 0.0f;
        }
    }

    // === 3. 速度模式逻辑 (你指定的) ===
    else if (current_mode == MODE_SPEED)
    {
        if (key == KEY_UP_SHORT)
        {
            // 逻辑：0 -> 30 -> 58
            if (target_val_spd < 1.0f)       target_val_spd = 30.0f; // 从0跳30
            else if (target_val_spd < 35.0f) target_val_spd = 58.0f; // 从30跳满速
        }
        
        if (key == KEY_DOWN_SHORT)
        {
            target_val_spd = 0.0f; // 急停
        }
    }

    // === 4. 位置模式逻辑 (你指定的) ===
    else if (current_mode == MODE_POS)
    {
        if (key == KEY_UP_SHORT)
        {
            is_auto_rotating = 0; // 打断自动演示
            target_val_pos += 90.0f; // 顺时针 90度
        }
        
        if (key == KEY_DOWN_SHORT)
        {
            is_auto_rotating = 0; 
            target_val_pos -= 90.0f; // 逆时针 90度
        }
        
        // 长按 UP：开启自动旋转演示 (像秒针一样)
        if (key == KEY_UP_LONG)
        {
            is_auto_rotating = 1; 
        }
        // 按一下 DOWN 停止自动旋转
        if (key == KEY_DOWN_LONG)
        {
            is_auto_rotating = 0;
        }
    }
}

// --- 控制循环 ---
void App_Ctrl_Loop_10ms(void)
{
    Motor_Update_State(&htim3);
    float pwm_final = 0.0f;

    // 处理 IDLE 模式下的点动停车逻辑
    if (current_mode == MODE_IDLE) {
        // 如果没有按键按下，就停车 (实现点动效果)
        // 这里简化处理，实际上 IDLE 模式下 App_Ctrl_KeyHandler 没被循环触发
        // 为了安全，IDLE 模式主要靠手拧或者简单的 PWM 测试
        // 实际上上面的 KeyHandler 里设置 Motor_Set_Force 在 10ms 后会被这里覆盖
        // 所以我们需要更底层的 Key 状态，不过为了简单，IDLE 模式先直接给 0
        // 如果你需要电动找零，最好在 While(1) 里检测按键状态直接发 PWM
        pwm_final = 0.0f; 
    }

    // 处理位置模式的自动旋转演示
    if (current_mode == MODE_POS && is_auto_rotating)
    {
        // 每 10ms 增加 0.9度 -> 相当于 90度/秒
        target_val_pos += 0.9f; 
    }

    switch (current_mode)
    {
        case MODE_IDLE:
            pwm_final = 0.0f;
            break;

        case MODE_SPEED:
            pwm_final = PID_Calc(&pid_spd, target_val_spd, g_MotorState.current_speed);
            break;

        case MODE_POS:
        {
            float err = target_val_pos - g_MotorState.total_angle;
            if (fabs(err) < 0.15f) 
            {
                pwm_final = 0.0f;
                pid_pos.error_sum = 0.0f;
                pid_spd.error_sum = 0.0f;
            }
            else
            {
                float want_speed = PID_Calc(&pid_pos, target_val_pos, g_MotorState.total_angle);
                pwm_final = PID_Calc(&pid_spd, want_speed, g_MotorState.current_speed);
                
                if (fabs(err) > 3.0f) {
                    if (want_speed > 0) pwm_final += 1000; else pwm_final -= 1000;
                } else if (fabs(err) > 0.1f) {
                    if (want_speed > 0) pwm_final += 200; else pwm_final -= 200;
                }
            }
            break;
        }
    }

    // 限幅 (58转电机不用太暴力，给 3599 足够了)
    if (pwm_final > 3599) pwm_final = 3599;
    if (pwm_final < -3599) pwm_final = -3599;

    Motor_Set_Force((int16_t)pwm_final);
    g_MotorState.current_pwm = (int16_t)pwm_final;
}
