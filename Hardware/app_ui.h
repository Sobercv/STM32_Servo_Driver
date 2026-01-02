#ifndef __APP_UI_H
#define __APP_UI_H

#include <stdint.h> // 为了识别 int16_t

// ==========================================
//    UI 刷新函数声明
// ==========================================
// 参数说明：
// tgt_pos: 目标位置 (Target Angle)
// cur_pos: 当前位置 (Current Angle)
// tgt_spd: 目标速度 (Target Speed - PID算出来的)
// cur_spd: 当前速度 (Current Speed - 测出来的)
// pwm:     当前占空比
// 只保留 3 个参数：目标位置、当前位置、PWM
void App_UI_Refresh(void);

#endif
