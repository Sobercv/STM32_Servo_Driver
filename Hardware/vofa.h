#ifndef __VOFA_H__
#define __VOFA_H__

// 引入 HAL 库，防止编译报错找不到 uint8_t 等类型
#include "main.h" 

// 函数声明：让外部(main.c)能调用它
void VOFA_JustFloat(float ch1, float ch2, float ch3);

#endif
