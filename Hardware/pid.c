#include "pid.h"
#include <math.h>

void PID_Init(PID_Config_t *pid, float kp, float ki, float kd, float limit) {
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    
    pid->error_sum = 0.0f;
    pid->error_last = 0.0f;
    pid->last_actual = 0.0f; // 初始化
    
    pid->output_limit = limit;
    pid->integral_limit = limit * 1.0f; 
    pid->dead_zone = 0.0f;
    
    // 默认积分范围，初始化后可以在 main 里手动修改
    pid->integral_range = 50000.0f; 
}

float PID_Calc(PID_Config_t *pid, float target, float current) {
    float output = 0.0f;
    float error = target - current;

    // ==========================================
    // 1. 死区判断 (修复版)
    // ==========================================
    // 如果进入死区，我们认为“误差为0”，
    // 但程序必须继续往下走，利用之前的积分值保持 PWM 输出！
    // 绝对不能 return 0.0f，也绝对不能清空积分！
    if (fabs(error) < pid->dead_zone) {
        error = 0.0f; 
        // 这里的技巧是：把 error 设为 0，P项输出就是 0。
        // 但是下面的 Ki * pid->error_sum 依然有值，
        // 这样电机就能保持匀速转动，不会停下来。
    }

    // 2. 积分分离
    if (fabs(error) < pid->integral_range) { 
        pid->error_sum += error;
        
        // 积分限幅
        if (pid->error_sum > pid->integral_limit)  pid->error_sum = pid->integral_limit;
        if (pid->error_sum < -pid->integral_limit) pid->error_sum = -pid->integral_limit;
    } else {
        pid->error_sum = 0.0f; 
    }

    // 3. 微分项 (Kd)
    float d_term = pid->Kd * (pid->last_actual - current);

    // 4. 计算总输出
    output = (pid->Kp * error) + (pid->Ki * pid->error_sum) + d_term;

    // 5. 更新历史数据
    pid->error_last = error;
    pid->last_actual = current; 

    // 6. 总输出限幅
    if (output > pid->output_limit)  output = pid->output_limit;
    if (output < -pid->output_limit) output = -pid->output_limit;

    return output;
}
