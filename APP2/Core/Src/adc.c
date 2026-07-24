/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
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
#include "adc.h"

/* USER CODE BEGIN 0 */
#include "modbus.h"
#include "stdio.h"
/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;

/* ADC1 init function */
void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* ADC1 clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PC0     ------> ADC1_IN10
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC1_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PC0     ------> ADC1_IN10
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0);

  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void ADC_VR_Test(void)
{

uint32_t adc_value = 0 ; 

HAL_ADC_Start(&hadc1); // ����ADC , ����һ�ι���һ�� 

          if(HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK )   //  �ȴ�adc ת������ 
            {
                    adc_value = HAL_ADC_GetValue(&hadc1) ; 
            }
        //adc_value = adc_value /10 ;  // 10����ƽ�� 
                            
        // 0         ----- 0  v 
        // 4095      ----- 3.3v
        // adc_value ----- val  v 
        float val = (adc_value*3.3/4096); 
        printf("%.2f ,%.2f \n",val,val);
			HAL_Delay(5);
}
void ADC_CPU_Read(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
        uint32_t ts_data = 0 ; 
        for(uint8_t i=0;i<10;i++)
        {
                HAL_ADC_Start(&hadc1); // 启动ADC , 启动一次工作一次 
                if(HAL_ADC_PollForConversion(&hadc1, 1000) == HAL_OK )   //  等待adc 转换结束 
                {
                        ts_data += HAL_ADC_GetValue(&hadc1) ; 
                }
        }
        ts_data = ts_data /10 ;  // 10次求平均
        // 0         -----0  v 
        // 4096      -----3.3v
        // adc_value -----val  v 
  float vsense  = ts_data * 3.3f / 4096.0f;
  float temp    = (vsense - 0.76f) / 0.0025f + 25.0f;
  REG_HOLD_BUF[7] =(uint16_t)(temp*100);
//         printf("CPU : %.1f C\n",temp);
//         sprintf((char * )str,"CPU : %.1f C",temp);
//         HAL_Delay(1000);
}
void ADC_CPU_Test(void)
{
  uint8_t str[32]={0};
  uint32_t ts_data = 0 ;
  for(uint8_t i=0;i<10;i++)
  {
    HAL_ADC_Start(&hadc1); // 启动ADC , 启动一次工作一次
    if(HAL_ADC_PollForConversion(&hadc1, 1000) == HAL_OK )   //  等待adc 转换结束
    {
      ts_data += HAL_ADC_GetValue(&hadc1) ;
    }

  }
  ts_data = ts_data /10 ;  // 10次求平均

  // 0         -----0  v
  // 4096      -----3.3v
  // adc_value -----val  v
  float vsense  = ts_data * 3.3f / 4096.0f;
  float temp    = (vsense - 0.76f) / 0.0025f + 25.0f;
  //        float temp=(1.43-ts_data*3.3/4095)/0.0043+25;
  printf("CPU : %.1f C\n",temp);
  sprintf((char * )str,"CPU : %.1f C",temp);
  HAL_Delay(1000);
}
/* USER CODE END 1 */
