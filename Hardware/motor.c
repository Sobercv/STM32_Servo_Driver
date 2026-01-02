#include "motor.h"
#include "tim.h"  // 引用 CubeMX 生成的 tim.h (包含 htim2, htim3 定义)
#include "gpio.h" // 引用引脚定义

// 定义全局实例
Motor_State_t g_MotorState = {0};

/**
  * @brief  电机初始化
  * @note   启动 PWM 和 编码器定时器
  */
void Motor_Init(void) {
    // 1. 启动编码器接口 (TIM3)
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    
    // 2. 启动 PWM 输出 (TIM2 Channel 1)
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    
    // 3. 初始状态：停车
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
    
    // 清除数据
    g_MotorState.total_pulses = 0;
    g_MotorState.total_angle = 0.0f;
    g_MotorState.current_speed = 0.0f;
    g_MotorState.current_pwm = 0;
}

// 静态变量保存历史值
static float filtered_speed = 0.0f; 

void Motor_Update_State(TIM_HandleTypeDef *htim_encoder) {
    // 1. 读取硬件计数
    // 现在因为是 10ms 读取一次，这个 delta_counts 可能是 4, 5, 6 这样比较大的数了
    int16_t delta_counts = (int16_t)__HAL_TIM_GET_COUNTER(htim_encoder);
    
    // 2. 累加位置
    g_MotorState.total_pulses += delta_counts;
    g_MotorState.total_angle = (float)g_MotorState.total_pulses / MOTOR_ENCODER_RESOLUTION * 360.0f;
    
    // 3. 计算原始速度
    // 注意：这里的 MOTOR_CTRL_PERIOD_S 必须已经在 motor.h 里改成了 0.01f
    float raw_speed = ((float)delta_counts / MOTOR_ENCODER_RESOLUTION) * (60.0f / MOTOR_CTRL_PERIOD_S);

    // ==========================================================
    // 4. 【轻量级滤波】
    // 系数改为 0.7f (之前为了测最大速度改成了 0.1f 或 0.5f)
    // 意思：新数据权重占 70%，反应非常灵敏！适合现在的 10ms 周期
    // ==========================================================
    filtered_speed = (filtered_speed * 0.3f) + (raw_speed * 0.7f);
    
    g_MotorState.current_speed = filtered_speed;

    // 5. 清零
    __HAL_TIM_SET_COUNTER(htim_encoder, 0);
}

/**
  * @brief  设置电机 PWM 和 方向
  * @param  pwm_duty: -7199 到 +7199
  */
void Motor_Set_Force(int16_t pwm_duty) {
    // 记录 PWM 值用于显示
    g_MotorState.current_pwm = pwm_duty;

    // 1. 处理反转 (负数)
    if (pwm_duty < 0) {
        // --- 反转逻辑 (AIN1=0, AIN2=1) ---
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
        
        // 取绝对值给 PWM 寄存器
        pwm_duty = -pwm_duty;
    } 
    // 2. 处理正转 (正数)
    else {
        // --- 正转逻辑 (AIN1=1, AIN2=0) ---
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
    }

    // 3. 硬件限幅保护 (防止超过 ARR 7199)
    if (pwm_duty > 3599) pwm_duty = 3599;

    // 4. 写入寄存器
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwm_duty);
}

// Hardware/motor.c

/**
  * @brief  重置编码器 (归零)
  * @note   将当前位置定义为机械零点
  */
void Motor_Reset_Encoder(void)
{
    // 1. 清除软件累加值
    g_MotorState.total_pulses = 0;
    g_MotorState.total_angle = 0.0f;
    
    // 2. 清除硬件定时器计数 (这一步很重要，否则下一次读取会有跳变)
    __HAL_TIM_SET_COUNTER(&htim3, 0); 
}
