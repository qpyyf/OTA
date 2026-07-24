/**
 * @file    i2c_soft.h
 * @brief   软件模拟 I2C 总线驱动头文件
 * @硬件平台 STM32F407 (WTM32F407)
 * @引脚    SCL = PD4, SDA = PD5
 * @说明    任意 GPIO 均可模拟，修改引脚宏即可移植
 */

#ifndef __I2C_SOFT_H
#define __I2C_SOFT_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/*==============================================================================
 * 用户配置区 —— 修改这里的宏定义可适配任意引脚
 *============================================================================*/

/* SCL: PD4 */
#define I2C_SCL_GPIO_PORT       GPIOD
#define I2C_SCL_PIN             GPIO_PIN_4
#define I2C_SCL_CLK_ENABLE()    __HAL_RCC_GPIOD_CLK_ENABLE()

/* SDA: PD5 */
#define I2C_SDA_GPIO_PORT       GPIOD
#define I2C_SDA_PIN             GPIO_PIN_5
#define I2C_SDA_CLK_ENABLE()    __HAL_RCC_GPIOD_CLK_ENABLE()

/* I2C 速率控制（调整 delay_us 的延时参数） */
#define I2C_DELAY_US            5       /* 标准模式约 100kHz，减小可提高速率 */

/*==============================================================================
 * GPIO 操作宏（无需修改）
 *============================================================================*/

#define I2C_SCL_H()     HAL_GPIO_WritePin(I2C_SCL_GPIO_PORT, I2C_SCL_PIN, GPIO_PIN_SET)
#define I2C_SCL_L()     HAL_GPIO_WritePin(I2C_SCL_GPIO_PORT, I2C_SCL_PIN, GPIO_PIN_RESET)
#define I2C_SDA_H()     HAL_GPIO_WritePin(I2C_SDA_GPIO_PORT, I2C_SDA_PIN, GPIO_PIN_SET)
#define I2C_SDA_L()     HAL_GPIO_WritePin(I2C_SDA_GPIO_PORT, I2C_SDA_PIN, GPIO_PIN_RESET)
#define I2C_SDA_READ()  HAL_GPIO_ReadPin(I2C_SDA_GPIO_PORT, I2C_SDA_PIN)

/*==============================================================================
 * 函数声明
 *============================================================================*/

void     I2C_Soft_Init(void);           /* GPIO 初始化 */
void     I2C_Start(void);               /* 起始信号 */
void     I2C_Stop(void);                /* 停止信号 */
void     I2C_SendByte(uint8_t data);    /* 发送 1 字节 */
uint8_t  I2C_ReadByte(void);            /* 读取 1 字节 */
uint8_t  I2C_WaitAck(void);             /* 等待应答 (0=ACK, 1=NACK) */
void     I2C_Ack(uint8_t ack);          /* 发送应答 (0=ACK, 1=NACK) */

#endif /* __I2C_SOFT_H */
