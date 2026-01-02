#include "app_ui.h"
#include "OLED.h"
#include "app_ctrl.h" 
#include "motor.h"    
#include <stdio.h>
#include <math.h>     

// --- 初始化 ---
void App_UI_Init(void)
{
    OLED_Init();
    OLED_Clear();
}

// --- 刷新界面 ---
void App_UI_Refresh(void)
{
    char buf[32]; 

    switch (current_mode)
    {
        // ------------------------------------------------
        // 1. 待机模式 (IDLE) - 毕设高端成品风
        // ------------------------------------------------
        case MODE_IDLE:
            // 使用双横线夹击，非常有工业感
            // 刚好16个字符宽度
            OLED_ShowString(0, 0,  "================", OLED_8X16);
            
            // 项目名称 (显得很专业)
            OLED_ShowString(0, 16, " DIGITAL  SERVO ", OLED_8X16);
            
            // 版本/状态 (不再显示 STOP，而是 SYSTEM READY)
            OLED_ShowString(0, 32, "  System Ready  ", OLED_8X16);
            
            OLED_ShowString(0, 48, "================", OLED_8X16);
            break;

        // ------------------------------------------------
        // 2. 速度模式 (SPEED) - 保持对齐
        // ------------------------------------------------
        case MODE_SPEED:
            OLED_ShowString(0, 0, "[SPEED LOOP]    ", OLED_8X16);
            
            // "Tgt:  30.0   rpm" (16字符)
            sprintf(buf, "Tgt: %5.1f   rpm", target_val_spd);
            OLED_ShowString(0, 16, buf, OLED_8X16);
            
            // "Cur:  29.9   rpm" (16字符)
            sprintf(buf, "Cur: %5.1f   rpm", g_MotorState.current_speed);
            OLED_ShowString(0, 32, buf, OLED_8X16);
            
            // "PWM:  1500      " (16字符)
            sprintf(buf, "PWM: %5d      ", g_MotorState.current_pwm);
            OLED_ShowString(0, 48, buf, OLED_8X16);
            break;

        // ------------------------------------------------
        // 3. 位置模式 (POS) - 修复括号显示
        // ------------------------------------------------
        case MODE_POS:
            OLED_ShowString(0, 0, "[POS LOOP]      ", OLED_8X16);
            
            // 视觉转换
            float show_tgt = fmod(target_val_pos, 360.0f);
            float show_cur = fmod(g_MotorState.total_angle, 360.0f);
            if (show_tgt < 0) show_tgt += 360.0f;
            if (show_cur < 0) show_cur += 360.0f;

            // 1. Set (16字符)
            // "Set: 090.0   Deg"
            sprintf(buf, "Set: %05.1f   Deg", show_tgt);
            OLED_ShowString(0, 16, buf, OLED_8X16);
            
            // 2. Now (16字符)
            // "Now: 090.1   Deg"
            sprintf(buf, "Now: %05.1f   Deg", show_cur);
            OLED_ShowString(0, 32, buf, OLED_8X16);
            
            // 3. Err (关键修改!)
            float err = target_val_pos - g_MotorState.total_angle;
            
            if (fabs(err) < 0.2f) {
                // 锁定状态: 减少一个空格，确保括号能显示
                // 原来: "Err: %5.1f   [OK]" (17字符 -> 溢出)
                // 现在: "Err: %5.1f  [OK]" (16字符 -> 完美)
                // 布局: "Err:  0.0  [OK]"
                sprintf(buf, "Err: %5.1f  [OK]", err); 
            } else {
                // 正常状态: 保持对齐
                // 布局: "Err: -0.1  Deg " (最后留空擦除残留)
                sprintf(buf, "Err: %5.1f  Deg ", err); 
            }
            OLED_ShowString(0, 48, buf, OLED_8X16);
            break;
    }

    OLED_Update();
}
