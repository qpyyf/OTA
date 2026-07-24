/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(jdq_GPIO_Port, jdq_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(beep_GPIO_Port, beep_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RS485_WR_GPIO_Port, RS485_WR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : key3_Pin */
  GPIO_InitStruct.Pin = key3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(key3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : key1_Pin */
  GPIO_InitStruct.Pin = key1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(key1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : jdq_Pin beep_Pin */
  GPIO_InitStruct.Pin = jdq_Pin|beep_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LED2_Pin LED1_Pin */
  GPIO_InitStruct.Pin = LED2_Pin|LED1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : key2_Pin */
  GPIO_InitStruct.Pin = key2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(key2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RS485_WR_Pin */
  GPIO_InitStruct.Pin = RS485_WR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(RS485_WR_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 2 */
void GPIO_Control(uint8_t device,uint8_t cmd){
	if(device==LED1)
	{
		if(cmd==ON)
		{
			HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);//led1(pb8)��ͨ����
		}
		if(cmd==OFF){
			HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
		}
	}
	else if(device==LED2){
		if(cmd==ON){
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
		}
		if(cmd==OFF){
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
			}
		}
	else if(device==beep)
		{
		if(cmd==OFF){
		HAL_GPIO_WritePin(beep_GPIO_Port, beep_Pin, GPIO_PIN_RESET);
		}
		if(cmd==ON){
		HAL_GPIO_WritePin(beep_GPIO_Port, beep_Pin, GPIO_PIN_SET);
			}
		}
		else if(device==jdq)
		{
		if(cmd==OFF){
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
		}
		if(cmd==ON){
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
			}
		}
	
}

uint8_t KeyScan(void)
{
	if(HAL_GPIO_ReadPin(key1_GPIO_Port,key1_Pin)==GPIO_PIN_SET)//key1按下变成低电平判断按下
	{
		return key1;
	}
	else if(HAL_GPIO_ReadPin(key2_GPIO_Port,key2_Pin)==GPIO_PIN_SET)
	{
		return key2;
	}
	else if(HAL_GPIO_ReadPin(key3_GPIO_Port,key3_Pin)==GPIO_PIN_SET)
	{
		return key3;
	}

		return 0;
}	
void KeyTest(void){
	uint8_t keyVal=KeyScan();
	if(keyVal==key1)
	{
		static uint8_t state=OFF;
		GPIO_Control(LED2,state);
		GPIO_Control(LED1,state);
		GPIO_Control(jdq,state);
		GPIO_Control(beep,state);
	}
	else if(keyVal==key3)
	{
		static uint8_t state=ON;
		GPIO_Control(LED1,state);
		GPIO_Control(LED2,state);
		GPIO_Control(jdq,state);
		GPIO_Control(beep,state);
	} 
}



void LED_Test(){
GPIO_Control(LED1,ON);
GPIO_Control(LED2,ON);
GPIO_Control(beep,ON);
GPIO_Control(jdq,ON);
HAL_Delay(3000);
GPIO_Control(LED1,ON);
GPIO_Control(LED2,OFF);
GPIO_Control(beep,ON);
GPIO_Control(jdq,OFF);
HAL_Delay(3000);
}




/* USER CODE END 2 */
