/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body (Modularized Version)
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
#include "motor.h"
#include "key.h"
#include "vofa.h"

#include "app_ui.h"
#include "app_ctrl.h"

#include <stdio.h>
#include <math.h> // fmod
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t lcd_timer = 0;  // 屏幕刷新计时
uint32_t vofa_timer = 0; // 上位机发送计时
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  // --- 硬件初始化 ---
  OLED_Init();
  OLED_Clear();
  Motor_Init();

  // 设定 PWM 频率为 20kHz (静音)
  __HAL_TIM_SET_AUTORELOAD(&htim2, 3599);

  // --- 软件模块初始化 ---
  App_Ctrl_Init(); // 初始化 PID 和变量

  // --- 绘制 UI 静态框架 ---
  OLED_ShowString(0, 0, "PID Servo", OLED_8X16);
  OLED_ShowString(0, 16, "Tgt:", OLED_8X16);
  OLED_ShowString(0, 32, "Cur:", OLED_8X16);
  OLED_ShowString(0, 48, "PWM:", OLED_8X16);
  OLED_Update();

  // --- 启动定时器中断 (心脏开始跳动) ---
  HAL_TIM_Base_Start_IT(&htim1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
      // =========================================================
      // 1. 按键处理 (只负责发令)
      // =========================================================
      uint8_t key = Key_Scan(); // 扫描按键

      // 情况 A: 短按 Mode 键 -> 切换模式
      if (key == KEY_MODE_SHORT) 
      {
          // 强制类型转换消除警告
          App_Ctrl_SetMode((SystemMode_t)(current_mode + 1)); 
          OLED_Clear(); // 刷屏
      }
      // 情况 B: 其他有效按键 (长按Mode, 短按/长按 Up/Down)
      else if (key != KEY_NONE) 
      {
          // 全部甩给 app_ctrl.c 里的逻辑去处理
          // 那里包含了：一键回原点、演示模式、速度跳变等逻辑
          App_Ctrl_KeyHandler(key); 
      }
      // =========================================================
      // 2. 屏幕显示 (只负责显示)
      // =========================================================
      if (HAL_GetTick() - lcd_timer > 100) 
      {
          lcd_timer = HAL_GetTick();
          App_UI_Refresh();
      }

      // =========================================================
      // 3. 上位机波形发送
      // =========================================================
      if (HAL_GetTick() - vofa_timer > 10) 
      {
          vofa_timer = HAL_GetTick();
          float send_tgt = (current_mode == MODE_POS) ? target_val_pos : target_val_spd;
          VOFA_JustFloat(send_tgt, g_MotorState.total_angle, g_MotorState.current_speed);
      }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
// =========================================================
// 4. 中断回调 (现在只负责调用 App_Ctrl)
// =========================================================
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    static uint8_t loop_divider = 0;

    if (htim->Instance == TIM1) 
    {
        // 10ms 分频
        loop_divider++;
        if (loop_divider < 10) return;
        loop_divider = 0;

        // 🔥 调用控制层核心 🔥
        App_Ctrl_Loop_10ms();
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
