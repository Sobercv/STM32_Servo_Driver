#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"

// ============================================
//   JGA25-370 (103:1) 电机关键参数修正
// ============================================

// 1. 霍尔编码器本身的物理线数 (电机尾部磁环的极数)
// 这款电机通常是 11 线 (11个脉冲/每转)
#define MOTOR_RAW_PULSES          11  

// 2. 减速箱的减速比 
// 你的电机贴纸上写的是 103:1 (或者接近 103)
#define MOTOR_GEAR_RATIO          103 

// 3. 定时器倍频模式 (STM32 编码器模式默认是 4 倍频)
// 它会统计 A相和 B相的 上升沿 + 下降沿，所以精度提高 4 倍
#define MOTOR_TIM_MULTIPLIER      4   

// 4. 【最终分辨率】轮子转一圈的总脉冲数 
// 公式：11 * 4 * 103 = 4532
#define MOTOR_ENCODER_RESOLUTION  (float)(MOTOR_RAW_PULSES * MOTOR_TIM_MULTIPLIER * MOTOR_GEAR_RATIO)

// 5. 控制周期 (单位：秒)
// 你的定时器 TIM1 是 10ms 中断一次，所以这里必须是 0.01
#define MOTOR_CTRL_PERIOD_S       0.01f 

// ... 结构体定义保持不变 ...
typedef struct {
    int32_t  total_pulses;   // 总脉冲数 (long long)
    float    total_angle;    // 总角度
    float    current_speed;  // 当前速度 (RPM)
    int16_t  current_pwm;    // 当前 PWM 值
} Motor_State_t;

extern Motor_State_t g_MotorState;

void Motor_Init(void);
void Motor_Update_State(TIM_HandleTypeDef *htim_encoder);
void Motor_Set_Force(int16_t pwm_duty);
void Motor_Reset_Encoder(void); 

#endif
