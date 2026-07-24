/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    iwdg.c
  * @brief   This file provides code for the configuration
  *          of the IWDG instances.
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
#include "iwdg.h"

/* USER CODE BEGIN 0 */
#include "stm32f4xx_it.h"
#include "gpio.h"
/* USER CODE END 0 */

IWDG_HandleTypeDef hiwdg;

/* IWDG init function */
void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_64;  // 32kHz/64=500Hz, 超时≈8.2秒(OTA擦除需要2~4秒)
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/* USER CODE BEGIN 1 */
void IWDG_Test(void)
{
   // OLED_ShowStr(16,0, (unsigned char*)"IWDG Test", 2); 
    // ��ȡ���� ESC , ���ESC �������¾�ι�� , ����ι��, ��ϵͳ��λ 
    if(Keynumber == 3) 
    {
        Keynumber = 0 ; // ��ֵ���Ϊ0 
        HAL_IWDG_Refresh(&hiwdg) ; // ι��ָ�� , 2���ϵͳ��λ 
		GPIO_Control(LED2,OFF);
        HAL_Delay(20);
		GPIO_Control(LED2,ON);
    }
    HAL_Delay(50); // ��ʱ500ms 
        
}
/* USER CODE END 1 */
