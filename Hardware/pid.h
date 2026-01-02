#ifndef __PID_H
#define __PID_H

#include <stdint.h>

// ==========================================
//    PID 对象结构体
// ==========================================
typedef struct {
    float Kp;
    float Ki;
    float Kd;
    
    float error_sum;   // 积分累计
    float error_last;  // 上次误差
    float last_actual; // 🔥新增：记录上次的实际值(用于微分优化)
    
    float output_limit;   // 总输出限幅
    float integral_limit; // 积分限幅
    float dead_zone;      // 死区
    float integral_range; // 🔥新增：积分分离的范围 (比如 <10度才积分)
} PID_Config_t;

// ==========================================
//    函数声明
// ==========================================

// 初始化 PID 参数
// limit: 总输出限幅 (比如PWM最大值7199，或者最大转速60)
void PID_Init(PID_Config_t *pid, float kp, float ki, float kd, float limit);

// 计算 PID 输出
// target: 目标值 (比如 90度)
// current: 当前值 (比如 88度)
float PID_Calc(PID_Config_t *pid, float target, float current);

// 重置 PID 状态 (比如从停止恢复运行时，清除之前的积分历史)
void PID_Reset(PID_Config_t *pid);

#endif

