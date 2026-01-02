#ifndef __ENCODER_H__
#define __ENCODER_H__

#include "main.h"

//对外接口函数

// 1.扫描旋钮状态（放在主循环里，建议无延时调用）
void Encoder_Scan(void);

// 2.获取当前设定的目标角度（给PID用）
float Encoder_Get_Target(void);
	
// 3.强制设置目标角度(比如系统复位使用)

void Encoder_Set_Target(float val);

#endif
