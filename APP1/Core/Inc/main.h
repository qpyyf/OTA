/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define key3_Pin GPIO_PIN_13
#define key3_GPIO_Port GPIOC
#define key3_EXTI_IRQn EXTI15_10_IRQn
#define key1_Pin GPIO_PIN_0
#define key1_GPIO_Port GPIOA
#define jdq_Pin GPIO_PIN_4
#define jdq_GPIO_Port GPIOA
#define beep_Pin GPIO_PIN_6
#define beep_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_2
#define LED2_GPIO_Port GPIOB
#define key2_Pin GPIO_PIN_8
#define key2_GPIO_Port GPIOE
#define key2_EXTI_IRQn EXTI9_5_IRQn
#define RS485_WR_Pin GPIO_PIN_15
#define RS485_WR_GPIO_Port GPIOD
#define LED1_Pin GPIO_PIN_8
#define LED1_GPIO_Port GPIOB

#define BootLoader_Size         0x8000U         // BootLoader的大小 32K  扇区0+扇区1 0x0800 0000--0x0800 7FFF
#define Application_1_Addr      0x08008000U     // 应用程序1的首地址 大小 224K（16k+16k+64k+128k）扇区2 3 4 5
/* USER CODE BEGIN Private defines */
#define DMA_BUF_SIZE   256
extern uint8_t RX1_Buf[]; // 接收缓冲区

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
