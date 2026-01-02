#include "key.h"
#include "gpio.h" // 引入 gpio.h 确保能找到 GPIOA 定义

// 定义长按阈值 (单位: ms)
#define LONG_PRESS_TIME 1000 

uint8_t Key_Scan(void)
{
    static uint32_t press_start_time = 0;
    static uint8_t  last_key_state = 0; // 0:松开, 1:按下
    static uint8_t  current_key_val = 0; // 记录按下的键值

    // ============================================================
    // 1. 读取物理引脚 (修正为 GPIOA)
    // ============================================================
    
    // 读取 MODE 键 (GPIOA)
    uint8_t val_mode = HAL_GPIO_ReadPin(GPIOA, KEY_MODE_Pin);
    
    // 读取 UP 键 (GPIOA)
    uint8_t val_up   = HAL_GPIO_ReadPin(GPIOA, KEY_UP_Pin);
    
    // 读取 DOWN 键 (GPIOA)
    uint8_t val_down = HAL_GPIO_ReadPin(GPIOA, KEY_DOWN_Pin);

    // 逻辑反转：因为开了上拉(Pull-Up)，按下是低电平(RESET)，没按是高电平(SET)
    uint8_t pressed = (val_mode == GPIO_PIN_RESET) || 
                      (val_up == GPIO_PIN_RESET)   || 
                      (val_down == GPIO_PIN_RESET);

    // ============================================================
    // 2. 状态机逻辑 (消抖 + 长短按判断)
    // ============================================================

    // [A] 按键刚刚按下
    if (pressed && last_key_state == 0)
    {
        HAL_Delay(10); // 消抖
        
        // 再次读取确认，防止干扰
        val_mode = HAL_GPIO_ReadPin(GPIOA, KEY_MODE_Pin);
        val_up   = HAL_GPIO_ReadPin(GPIOA, KEY_UP_Pin);
        val_down = HAL_GPIO_ReadPin(GPIOA, KEY_DOWN_Pin);
        
        if (val_mode == GPIO_PIN_RESET)      current_key_val = 1;
        else if (val_up == GPIO_PIN_RESET)   current_key_val = 2;
        else if (val_down == GPIO_PIN_RESET) current_key_val = 3;
        else return KEY_NONE; // 可能是抖动

        press_start_time = HAL_GetTick(); // 记录按下时刻
        last_key_state = 1;
        return KEY_NONE;
    }

    // [B] 按键刚刚松开 (触发事件)
    if (!pressed && last_key_state == 1)
    {
        last_key_state = 0;
        uint32_t press_duration = HAL_GetTick() - press_start_time;
        
        uint8_t event = KEY_NONE;

        if (press_duration > LONG_PRESS_TIME) // 长按事件
        {
            if (current_key_val == 1) event = KEY_MODE_LONG;
            if (current_key_val == 2) event = KEY_UP_LONG;
            if (current_key_val == 3) event = KEY_DOWN_LONG;
        }
        else // 短按事件
        {
            if (current_key_val == 1) event = KEY_MODE_SHORT;
            if (current_key_val == 2) event = KEY_UP_SHORT;
            if (current_key_val == 3) event = KEY_DOWN_SHORT;
        }
        return event;
    }

    return KEY_NONE;
}
