#ifndef __KEY_H
#define __KEY_H

#include "main.h"

// 定义按键事件
#define KEY_NONE       0
#define KEY_MODE_SHORT 1
#define KEY_MODE_LONG  2
#define KEY_UP_SHORT   3
#define KEY_UP_LONG    4
#define KEY_DOWN_SHORT 5
#define KEY_DOWN_LONG  6

// 函数声明
uint8_t Key_Scan(void);

#endif

