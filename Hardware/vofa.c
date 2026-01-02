#include "vofa.h"
#include "usart.h" // 必须引入这个，因为要用到 huart1

// 具体的实现代码放在这里
void VOFA_JustFloat(float ch1, float ch2, float ch3)
{
    // 定义发送缓冲区：3个数据 * 4字节 + 4字节帧尾 = 16字节
    uint8_t byte[16]; 
    
    // 帧尾 (JustFloat 协议)
    uint8_t tail[4] = {0x00, 0x00, 0x80, 0x7f}; 

    // 利用指针把 float 转成字节
    float *p = (float *)byte;
    p[0] = ch1; // 第1条线
    p[1] = ch2; // 第2条线
    p[2] = ch3; // 第3条线

    // 填入帧尾
    byte[12] = tail[0];
    byte[13] = tail[1];
    byte[14] = tail[2];
    byte[15] = tail[3];

    // 发送！
    // 这里的 huart1 来自 usart.h
    HAL_UART_Transmit(&huart1, byte, 16, 10);
}
