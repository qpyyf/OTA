/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "can.h"
#include "crc.h"
#include "dma.h"
#include "i2c.h"
#include "iwdg.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "wwdg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "AHT20.h"
#include "FreeRTOS_Demo.h"
#include "ina226.h"
#include "sd3078.h"
#include "mb.h"
#include "modbus.h"
#include "wifi4g.h"
#include "mqtt.h"
#include "canfestival.h"
#include "flash.h"
#include "testslave.h"
#include "../../Core/Inc/can.h"
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
  /*************************** 中断向量表动态偏移 *******************************/
  // 优先级: 看门狗恢复标志(-8) > 运行标志(-4) > 默认APP2
  uint32_t app1_wdt_flag = MyFlash_ReadWord(Application_1_Addr + Application_Size - 8);
  uint32_t app2_wdt_flag = MyFlash_ReadWord(Application_2_Addr + Application_Size - 8);
  /*************************** 看门狗恢复标志检查 *******************************/
  if (app1_wdt_flag == 0x55555555) {
    SCB->VTOR = FLASH_BASE | Application_1_Addr;
  } else if (app2_wdt_flag == 0x55555555) {
    SCB->VTOR = FLASH_BASE | Application_2_Addr;
  } else {
  /*************************** 运行标志检查 *******************************/
    uint32_t app1_run_flag = MyFlash_ReadWord(Application_1_Addr + Application_Size - 4);
    uint32_t app2_run_flag = MyFlash_ReadWord(Application_2_Addr + Application_Size - 4);

    if (app1_run_flag == 0xAAAAAAAA) {
      SCB->VTOR = FLASH_BASE | Application_1_Addr;
    } else if (app2_run_flag == 0xAAAAAAAA) {
      SCB->VTOR = FLASH_BASE | Application_2_Addr;
    } else {
      SCB->VTOR = FLASH_BASE | Application_2_Addr;  // 默认APP2
    }
  }
  __enable_irq();
  /*************************** 中断向量表偏移完成 *******************************/
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();
  MX_IWDG_Init();
  /* USER CODE BEGIN SysInit */
  /***************************自动偏移中断向量表地址（根据app1或app2的更新标志位来判断）*******************************/
  


  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_TIM4_Init();
  MX_I2C2_Init();
  // MX_WWDG_Init();
  MX_RTC_Init();
  MX_ADC1_Init();

  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM3_Init();
  MX_TIM5_Init();
  MX_CAN1_Init();
  MX_TIM2_Init();
  MX_CRC_Init();
  HAL_IWDG_Refresh(&hiwdg) ; // 刷新看门狗
  /* USER CODE BEGIN 2 */
 // HAL_UART_Receive_IT(&huart1, RX1_Buf , 16); // 使能串UART1接收中断

  /*
  //定时器5 用于10ms中断,//                             不自动重装计数器
  //定时器2 用于1us中断, 用于更新时间
  //定时器3用于500ms中断, 用于modbus通信更新传感器数据时间   自动重装计数器
  //定时器4 用于modbus通信50us定时器//  被modbus使用
  //定时器7 用于hal 1ms中断, 用于更新时间
  //定时器sysTick 用于freeRTOS系统时间更新
  //串口3 用于modbus通信
  //串口2 用于wifi4g（mqtt）通信
  //串口1 用于主机通信
  */
  // RTC_Time_t t = {
  //       .sec   =  0,
  //       .min   = 46,
  //       .hour  = 0,
  //       .week  =  6,                  // 会自动修正，任意值即可
  //       .day   = 17,
  //       .month =  5,
  //       .year  = 2026 % 100
  //   };

  printf("MQTT TEST\n");
  // printf("PWR Sleep Test\n");
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE); //使能串UART1 IDLE(空闲)中断
  HAL_UART_Receive_DMA(&huart1, RX1_Buf, DMA_BUF_SIZE); //设置DMA传输，将uart1的数据搬运到RX1_Buf中
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE); //使能串UART2 IDLE(空闲)中断
  HAL_UART_Receive_DMA(&huart2, RX2_Buf, DMA_BUF_SIZE); //设置DMA传输，将uart2的数据搬运到RX2_Buf中
  // OLED_Init();
  // OLED_Fill(0x00);//全屏灭
  AHT20_Init();
  INA226_Init();

  // SD3078_SetTime((RTC_Time_t *)&t);
  // RTC_SetDateTime(26,5,17,0,46,0);
  /*************************** 写入运行标志(仅首次启动) *******************************/
  // Flash物理限制: 只能1→0, 0→1需擦除. 0x00000000无法写回0xAAAAAAAA
  // 因此仅在擦除态(0xFFFFFFFF)时写入, OTA后已写好/看门狗恢复时则跳过
  if (MyFlash_ReadWord(Application_1_Addr + Application_Size - 4) == 0xFFFFFFFF) {
    uint32_t update1_flag = 0xAAAAAAAA;
    MyFlash_Save(Application_1_Addr + Application_Size - 4, (char *)&update1_flag, 4);
  }
  /*************************** 运行标志写入完成 *******************************/
  FreeRTOS_Start();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // eMBPoll();
//     Modbus_parse();
// 	  ADC_CPU_Test();
// 	  AHT20_Test();
// //	  ADC_VR_Test();
// 	  RTC_Test();
// 	  //HAL_Delay(1000);
// 	  SD3078_GetTime((RTC_Time_t *)&t);
// 	 printf("%04d-%02d-%02d %02d:%02d:%02d\n",2000+t.year,t.month,t.day,
// t.hour,t.min,t.sec);
	  
	  
//	GPIO_Control(LED1,OFF); // LED1 亮 
//    HAL_Delay(2000); //  延时2秒 
//    GPIO_Control(LED1,ON); // LED1 灭
//    printf("System is Running\n");
//    // 使用 LED2指示, 系统进入睡眠模式 
//   // GPIO_Control(LED2,ON);  // LED2 亮 
//    HAL_SuspendTick() ;// 暂停 滴答定时器 ， 因为嘀嗒定时器会产生1ms的中断， 必须关掉
//    printf("System is sleeping\n");
//    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON,PWR_SLEEPENTRY_WFI) ; // 进入睡眠模式 
//    // 等待被唤醒 ........  , 可以按下6个按键中的任何一个即可唤醒 , 最好按下ESC,因为ESC默认动作时关闭蜂鸣器
//    // 被唤醒后 
//    printf("System is wakeup\n");
//  //  GPIO_Control(LED1,OFF);  // LED2 灭
//    HAL_ResumeTick(); //  恢复滴答定时器的计时
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */


	  
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM7)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
  if(htim->Instance == TIM3) {
    TIM3_Timerout_Flag=1;
    static uint16_t MQTTsenior_updata=0;
    MQTTsenior_updata++;
    if (MQTTsenior_updata>10) {
      MQTTsenior_updata=0;
      MQTTUpLoad_flag=1;
    }
  }
  if (htim->Instance == TIM5) {
    HAL_TIM_Base_Stop(&htim5);
    WIFI4G_CMD_Status = WIFI4G_Parse_Queue(&QUART2);
  }
  /* USER CODE END Callback 1 */
}

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

#ifdef  USE_FULL_ASSERT
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