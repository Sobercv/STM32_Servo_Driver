#include "encoder.h"

// 硬件引脚定义
#define EC11_A_PORT   GPIOB
#define EC11_A_PIN    GPIO_PIN_14
#define EC11_B_PORT   GPIOB
#define EC11_B_PIN    GPIO_PIN_15
#define EC11_SW_PORT  GPIOA
#define EC11_SW_PIN   GPIO_PIN_8

// --- 变量定义 ---
static volatile float target_angle = 0.0f; 
static uint32_t last_turn_time = 0;
static uint8_t old_state = 0; // 上一次的状态

// --- 状态表 (还是那个表，但用法变了) ---
const int8_t encoder_table[16] = {
    0,  1, -1,  0,  // 00 -> 00, 01, 10, 11
   -1,  0,  0,  1,  // 01 -> 00, 01, 10, 11
    1,  0,  0, -1,  // 10 -> 00, 01, 10, 11
    0, -1,  1,  0   // 11 -> 00, 01, 10, 11
};

float Encoder_Get_Target(void) { return target_angle; }
void Encoder_Set_Target(float val) { target_angle = val; }

/**
  * @brief  即时响应扫描 (每 1ms 调用)
  * 不等待完整周期，有动静就加减，解决"不动"的问题
  */
void Encoder_Scan(void)
{
    // 1. 读取 A / B 当前电平
    uint8_t a = HAL_GPIO_ReadPin(EC11_A_PORT, EC11_A_PIN);
    uint8_t b = HAL_GPIO_ReadPin(EC11_B_PORT, EC11_B_PIN);

    // 2. 组合当前状态
    uint8_t cur_state = (a << 1) | b;

    // 3. 查表看发生了什么变化
    // 索引 = (旧状态 << 2) | 新状态
    uint8_t index = (old_state << 2) | cur_state;
    
    // 获取变化量 (-1, 0, +1)
    int8_t delta = encoder_table[index];
    
    // 更新旧状态，供下一次比对
    old_state = cur_state;

    // 4. 【核心改动】只要 delta 不为 0，立刻触发改变！
    // 抛弃 pulses_sum >= 4 的限制
    if (delta != 0) 
    {
        // 速度自适应
        float step_per_pulse = 1.0f; // 基础步进改小一点，因为现在反应更灵敏了
        
        if (HAL_GetTick() - last_turn_time < 50) {
            step_per_pulse = 5.0f; // 加速
        }
        last_turn_time = HAL_GetTick();

        // 直接加上变化量 (delta 是 1 或 -1)
        target_angle += (delta * step_per_pulse);
    }

    // 5. 按键检测 (保持不变)
    static uint8_t sw_last = 1;
    static uint8_t sw_cnt = 0;
    uint8_t sw = HAL_GPIO_ReadPin(EC11_SW_PORT, EC11_SW_PIN);
    
    if (sw == 0) sw_cnt++;
    else sw_cnt = 0;
    
    if (sw_cnt > 20) { 
        if (sw_last == 1) {
            target_angle = 0.0f;
            sw_last = 0;
        }
    } else if (sw == 1) {
        sw_last = 1;
    }
}
