/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "can_timers.h"
#include "GPIO.h"
#include "stdio.h"
#include "USART.h"
#include "modbus.h"
#include "fifo.h"
#include "wifi4g.h"
#include "tim.h"
#include "../CanFestival/Inc/can_timers.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint8_t Keynumber;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern CAN_HandleTypeDef hcan1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern UART_HandleTypeDef huart2;
extern WWDG_HandleTypeDef hwwdg;
extern TIM_HandleTypeDef htim7;

/* USER CODE BEGIN EV */
void USER_UART_IRQHandler(UART_HandleTypeDef *huart);
/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
// void SVC_Handler(void)
// {
//   /* USER CODE BEGIN SVCall_IRQn 0 */
// // // // // // // // // // //
//   /* USER CODE END SVCall_IRQn 0 */
//   /* USER CODE BEGIN SVCall_IRQn 1 */
// // // // // // // // // // //
//   /* USER CODE END SVCall_IRQn 1 */
// }

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
// void PendSV_Handler(void)
// {
//   /* USER CODE BEGIN PendSV_IRQn 0 */
// // // // // // // // // // // //
//   /* USER CODE END PendSV_IRQn 0 */
//   /* USER CODE BEGIN PendSV_IRQn 1 */
// // // // // // // // // // // //
//   /* USER CODE END PendSV_IRQn 1 */
// }

/**
  * @brief This function handles System tick timer.
  */
// void SysTick_Handler(void)
// {
//   /* USER CODE BEGIN SysTick_IRQn 0 */
// // // // // // // // // // //
//   /* USER CODE END SysTick_IRQn 0 */
//
//   /* USER CODE BEGIN SysTick_IRQn 1 */
// // // // // // // // // // //
//   /* USER CODE END SysTick_IRQn 1 */
// }

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles Window watchdog interrupt.
  */
void WWDG_IRQHandler(void)
{
  /* USER CODE BEGIN WWDG_IRQn 0 */

  /* USER CODE END WWDG_IRQn 0 */
  HAL_WWDG_IRQHandler(&hwwdg);
  /* USER CODE BEGIN WWDG_IRQn 1 */

  /* USER CODE END WWDG_IRQn 1 */
}

/**
  * @brief This function handles DMA1 stream5 global interrupt.
  */
void DMA1_Stream5_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream5_IRQn 0 */

  /* USER CODE END DMA1_Stream5_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart2_rx);
  /* USER CODE BEGIN DMA1_Stream5_IRQn 1 */

  /* USER CODE END DMA1_Stream5_IRQn 1 */
}

/**
  * @brief This function handles CAN1 RX0 interrupts.
  */
void CAN1_RX0_IRQHandler(void)
{
  /* USER CODE BEGIN CAN1_RX0_IRQn 0 */

  /* USER CODE END CAN1_RX0_IRQn 0 */
  HAL_CAN_IRQHandler(&hcan1);
  /* USER CODE BEGIN CAN1_RX0_IRQn 1 */

  /* USER CODE END CAN1_RX0_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[9:5] interrupts.
  */
void EXTI9_5_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI9_5_IRQn 0 */

  /* USER CODE END EXTI9_5_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(key2_Pin);
  /* USER CODE BEGIN EXTI9_5_IRQn 1 */

  /* USER CODE END EXTI9_5_IRQn 1 */
}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */

  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */
  TimeDispatch();
  /* USER CODE END TIM2_IRQn 1 */
}

/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM3_IRQHandler(void)
{
  /* USER CODE BEGIN TIM3_IRQn 0 */

  /* USER CODE END TIM3_IRQn 0 */
  HAL_TIM_IRQHandler(&htim3);
  /* USER CODE BEGIN TIM3_IRQn 1 */

  /* USER CODE END TIM3_IRQn 1 */
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */

  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */
  USER_UART_IRQHandler(&huart2);
  /* USER CODE END USART2_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI15_10_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI15_10_IRQn 0 */

  /* USER CODE END EXTI15_10_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(key3_Pin);
  /* USER CODE BEGIN EXTI15_10_IRQn 1 */

  /* USER CODE END EXTI15_10_IRQn 1 */
}

/**
  * @brief This function handles TIM5 global interrupt.
  */
void TIM5_IRQHandler(void)
{
  /* USER CODE BEGIN TIM5_IRQn 0 */

  /* USER CODE END TIM5_IRQn 0 */
  HAL_TIM_IRQHandler(&htim5);
  /* USER CODE BEGIN TIM5_IRQn 1 */

  /* USER CODE END TIM5_IRQn 1 */
}

/**
  * @brief This function handles TIM7 global interrupt.
  */
void TIM7_IRQHandler(void)
{
  /* USER CODE BEGIN TIM7_IRQn 0 */

  /* USER CODE END TIM7_IRQn 0 */
  HAL_TIM_IRQHandler(&htim7);
  /* USER CODE BEGIN TIM7_IRQn 1 */

  /* USER CODE END TIM7_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream2 global interrupt.
  */
void DMA2_Stream2_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream2_IRQn 0 */

  /* USER CODE END DMA2_Stream2_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart1_rx);
  /* USER CODE BEGIN DMA2_Stream2_IRQn 1 */

  /* USER CODE END DMA2_Stream2_IRQn 1 */
}

/* USER CODE BEGIN 1 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
		if(GPIO_Pin==key1_Pin)
		{
			printf("key1 is ok\n");
			GPIO_Control(LED1,OFF);
			Keynumber=1;
		}
		if(GPIO_Pin==key2_Pin)
		{
			printf("key2 is ok\n");
			GPIO_Control(LED1,ON);
			GPIO_Control(jdq,ON);
			Keynumber=2;
		}
		if(GPIO_Pin==key3_Pin)
		{
			printf("key3 is ok\n");
			//GPIO_Control(jdq,OFF);
			Keynumber=3;
		}
}

// DMA接收半完成中断处理函数
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
        if(huart->Instance == USART1)//串口一
        {
            uint8_t Length  =  DMA_BUF_SIZE/2 - RX1_Offset ;
            //printf("HLength=%d\n",Length);
                #if 1
                HAL_UART_Transmit(&huart2,RX1_Buf+RX1_Offset,Length,HAL_MAX_DELAY);
                #else
                if (Enqueue_Bytes(&QUART1,RX1_Buf+RX1_Offset,Length) < 0) {
                  printf("Fifo Full\r\n");
                }
                #endif

            RX1_Offset += Length;
        }

        if(huart->Instance == USART2)//串口二
        {
          uint8_t Length  =  DMA_BUF_SIZE/2 - RX2_Offset ;
          //printf("HLength=%d\n",Length);
#if 0
          HAL_UART_Transmit(&huart1,RX2_Buf+RX2_Offset,Length,HAL_MAX_DELAY);
#else
          //HAL_UART_Transmit(&huart1,RX2_Buf+RX2_Offset,Length,HAL_MAX_DELAY);
          if (Enqueue_Bytes(&QUART2,RX2_Buf+RX2_Offset,Length) < 0) {
            printf("Fifo Full\r\n");
          }
#endif
          RX2_Offset += Length;
        }
}
//dma接收完成中断处理函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
     if(huart->Instance == USART1)
     {
        uint8_t Length  =  DMA_BUF_SIZE - RX1_Offset ;
#if 1
       HAL_UART_Transmit(&huart2,RX1_Buf+RX1_Offset,Length,HAL_MAX_DELAY);
#else
      if (Enqueue_Bytes(&QUART1,RX1_Buf+RX1_Offset,Length) < 0) {
        printf("Fifo Full\r\n");
      }
#endif
        //printf("CLength=%d\n",Length);
        RX1_Offset = 0 ; // 重置dma接收指移量
      }

    if(huart->Instance == USART2)
    {
      uint8_t Length  =  DMA_BUF_SIZE - RX2_Offset ;
#if 0
      HAL_UART_Transmit(&huart1,RX2_Buf+RX2_Offset,Length,HAL_MAX_DELAY);
#else
      //HAL_UART_Transmit(&huart1,RX2_Buf+RX2_Offset,Length,HAL_MAX_DELAY);
      if (Enqueue_Bytes(&QUART2,RX2_Buf+RX2_Offset,Length) < 0) {
        printf("Fifo Full\r\n");
      }
#endif
      //printf("CLength=%d\n",Length);
      RX2_Offset = 0 ; // 重置dma接收指移量
    }
}
// 用户自定义的函数 ， 处理串口空闲中断
void USER_UART_IRQHandler(UART_HandleTypeDef *huart) {
  if(huart->Instance == USART1)
  {
    if((__HAL_UART_GET_FLAG(huart, UART_FLAG_FE) ||
            __HAL_UART_GET_FLAG(huart, UART_FLAG_NE) )== SET )   //判断是否是帧错误或者噪音中断
    {
      __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_FE);
      __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_NE);
      huart->ErrorCode &= HAL_UART_ERROR_FE; // 清除错误标志位
      huart->ErrorCode &= HAL_UART_ERROR_NE; // 清除错误标志位
      HAL_UART_DMAStop(huart); // 关闭DMA
      HAL_UART_Receive_DMA(huart,RX1_Buf,DMA_BUF_SIZE);// 重新启动DMA
    }
    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET )   //判断是否是空闲中断
    {
      __HAL_UART_CLEAR_IDLEFLAG(huart);    //清除空闲中断标志（否则会一直不断进入中断）
      //计算接收到的数据长度 : BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx)
      uint8_t Length  =  DMA_BUF_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx) - RX1_Offset;
#if 0
      //  RS485_WR ENABLE 高电平  发送模式
      HAL_GPIO_WritePin(RS485_WR_GPIO_Port, RS485_WR_Pin, GPIO_PIN_SET);
      HAL_UART_Transmit(&huart2,RX1_Buf+RX1_Offset,Length,HAL_MAX_DELAY);
      // 低电平  默认为接收模式
      HAL_GPIO_WritePin(RS485_WR_GPIO_Port, RS485_WR_Pin, GPIO_PIN_RESET);
#else
      HAL_UART_Transmit(&huart2,RX1_Buf+RX1_Offset,Length,HAL_MAX_DELAY);
#endif
      RX1_Offset += Length;
      //printf("ILength=%d\n",Length);
    }
  }
   if(huart->Instance == USART2)
  {
    /* UART frame error interrupt occurred --------------------------------------*/
    if((__HAL_UART_GET_FLAG(huart, UART_FLAG_FE) ||
            __HAL_UART_GET_FLAG(huart, UART_FLAG_NE) )== SET )   //判断是否是帧错误或者噪音中断
    {
      __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_FE);
      __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_NE);
      huart->ErrorCode &= HAL_UART_ERROR_FE; // 清除错误标志位
      huart->ErrorCode &= HAL_UART_ERROR_NE; // 清除错误标志位
      HAL_UART_DMAStop(huart); // 关闭DMA
      HAL_UART_Receive_DMA(huart,RX2_Buf,DMA_BUF_SIZE);// 重新启动DMA
    }
    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET )   //判断是否是空闲中断
    {
      __HAL_UART_CLEAR_IDLEFLAG(huart);    //清除空闲中断标志（否则会一直不断进入中断）
      //计算接收到的数据长度 : BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart2_rx)
      uint8_t Length  =  DMA_BUF_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart2_rx) - RX2_Offset;
#if 0
      HAL_UART_Transmit(&huart1,RX3_Buf+RX3_Offset,Length,HAL_MAX_DELAY);
#else
      //HAL_UART_Transmit(&huart1,RX2_Buf+RX2_Offset,Length,HAL_MAX_DELAY);
      if(Enqueue_Bytes(&QUART2,RX2_Buf+RX2_Offset,Length) <0){
        printf("FIFO is Full");
      }
      RX2_Offset += Length;
      WIFI4G_CMD_Status = WIFI4G_Parse_Queue(&QUART2);
#endif

      //printf("ILength=%d\n",Length);
    }
  }
}

void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *hwwdg)
{
static  uint32_t  wwdg_count = 0 ; 
wwdg_count ++; // 50ms 加1 
// 获取按键 3 , 3 按键按下就喂狗 , 否则不喂狗, 让系统复位 
if(Keynumber == 3) 
{
Keynumber = 0 ; // 键值清空为0 
wwdg_count = 0 ; // 会让定时器可以再多工作2秒 
}
if(wwdg_count >=0)  // 2秒 
{
//  中断处理函数中喂狗
HAL_WWDG_Refresh(hwwdg) ; // 喂狗指令  
}
 else if(wwdg_count >40) // 2秒后 , 不喂狗 就会复位
        {
                    
        }
}
// void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
//   //产生500ms中断, 用于更新时间
//   if(htim->Instance == TIM3) {
//     TIM3_Timerout_Flag=1;
//   }
// }

void USART1_IRQHandler(void)
{
  // if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_ORE) != RESET ||
  //     __HAL_UART_GET_FLAG(&huart1, UART_FLAG_FE)  != RESET ||
  //     __HAL_UART_GET_FLAG(&huart1, UART_FLAG_NE)  != RESET)
  // {
  //   __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_ORE);
  //   __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_FE);
  //   __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_NE);
  //   __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_PE);
  //   return;
  // }
  HAL_UART_IRQHandler(&huart1);
  USER_UART_IRQHandler(&huart1);
}

/* USER CODE END 1 */
