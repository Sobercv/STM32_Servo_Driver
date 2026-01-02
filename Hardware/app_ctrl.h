#ifndef __APP_CTRL_H
#define __APP_CTRL_H

#include "main.h"

// 模式定义搬到这里
typedef enum {
    MODE_IDLE = 0,
    MODE_SPEED = 1,
    MODE_POS = 2
} SystemMode_t;

// 把全局变量封装起来，不让别人随便改，只通过函数操作
extern volatile SystemMode_t current_mode;
extern float target_val_spd;
extern float target_val_pos;

// 公开给 Main 调用的 API
void App_Ctrl_Init(void);          // 初始化 PID 和变量
void App_Ctrl_Loop_10ms(void);     // 放进定时器中断里的逻辑
void App_Ctrl_SetMode(SystemMode_t new_mode); // 切换模式的逻辑
void App_Ctrl_KeyHandler(uint8_t key); // 按键处理逻辑

#endif
