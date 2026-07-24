/**
 * @file    i2c_soft.c
 * @brief   软件模拟 I2C 总线驱动实现
 * @协议    I2C 标准时序（起始/停止/字节传输/应答）
 * @说明    完全软件模拟，不依赖硬件 I2C 外设
 */

#include "i2c_soft.h"

/*==============================================================================
 * 微秒级延时函数
 *
 * 使用 DWT 数据观察点跟踪单元实现精确微秒延时
 * 需在 main() 开头调用 DWT_Init() 初始化
 *============================================================================*/

static void I2C_Delay(void)
{
    uint32_t cnt = I2C_DELAY_US * (SystemCoreClock / 1000000) / 4;
    while (cnt--);
}

/*==============================================================================
 * I2C GPIO 初始化
 *
 * 将 SCL 和 SDA 配置为开漏输出，外接上拉电阻
 *============================================================================*/
void I2C_Soft_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 使能 GPIO 时钟 */
    I2C_SCL_CLK_ENABLE();
    I2C_SDA_CLK_ENABLE();

    /* 配置 SCL 和 SDA 为开漏输出 */
    GPIO_InitStruct.Pin   = I2C_SCL_PIN | I2C_SDA_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* 释放总线，拉高到空闲电平 */
    I2C_SCL_H();
    I2C_SDA_H();
}

/*==============================================================================
 * I2C 起始信号
 *
 * 时序: SCL 高电平时，SDA 从高 → 低跳变
 *             ______
 *  SDA  _____/      \______
 *             ______
 *  SCL  _____/      \______
 *============================================================================*/
void I2C_Start(void)
{
    I2C_SDA_H();
    I2C_Delay();
    I2C_SCL_H();
    I2C_Delay();
    I2C_SDA_L();            /* SCL 高时 SDA 变低 = 起始信号 */
    I2C_Delay();
    I2C_SCL_L();            /* 钳住总线，准备传输 */
}

/*==============================================================================
 * I2C 停止信号
 *
 * 时序: SCL 高电平时，SDA 从低 → 高跳变
 *                   ______
 *  SDA  __________/
 *             ______
 *  SCL  _____/      \______
 *============================================================================*/
void I2C_Stop(void)
{
    I2C_SDA_L();
    I2C_Delay();
    I2C_SCL_H();
    I2C_Delay();
    I2C_SDA_H();            /* SCL 高时 SDA 变高 = 停止信号 */
    I2C_Delay();
}

/*==============================================================================
 * 发送 1 字节（高位先行 MSB First）
 *
 * 每个 bit：SCL 低时设置 SDA 电平，SCL 高时从机采样
 *============================================================================*/
void I2C_SendByte(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        I2C_SCL_L();
        I2C_Delay();

        if (data & 0x80)
            I2C_SDA_H();
        else
            I2C_SDA_L();

        data <<= 1;
        I2C_Delay();
        I2C_SCL_H();        /* 从机在 SCL 上升沿采样 */
        I2C_Delay();
    }
    I2C_SCL_L();            /* 拉低 SCL，准备 ACK 阶段 */
}

/*==============================================================================
 * 读取 1 字节（高位先行 MSB First）
 *
 * 每个 bit：SCL 高时从 SDA 读取电平
 *============================================================================*/
uint8_t I2C_ReadByte(void)
{
    uint8_t data = 0;

    I2C_SDA_H();            /* 释放 SDA，由从机控制 */
    for (uint8_t i = 0; i < 8; i++)
    {
        data <<= 1;
        I2C_SCL_H();
        I2C_Delay();
        if (I2C_SDA_READ())
            data |= 0x01;
        I2C_SCL_L();
        I2C_Delay();
    }
    return data;
}

/*==============================================================================
 * 等待从机应答
 *
 * 在第 9 个 SCL 时钟检测 SDA 电平：
 *   0 = ACK  (SDA 被从机拉低)
 *   1 = NACK (SDA 保持高，或超时)
 *============================================================================*/
uint8_t I2C_WaitAck(void)
{
    uint16_t timeout = 1000;

    I2C_SDA_H();            /* 释放 SDA，让从机控制 */
    I2C_Delay();
    I2C_SCL_H();
    I2C_Delay();

    while (I2C_SDA_READ())  /* 等待从机拉低 SDA */
    {
        if (--timeout == 0)
        {
            I2C_Stop();
            return 1;       /* 超时 = NACK */
        }
    }

    I2C_SCL_L();
    return 0;               /* 收到 ACK */
}

/*==============================================================================
 * 主机发送应答/非应答
 *
 * 读数据时使用：
 *   ack = 0 → 发 ACK  （还要继续读）
 *   ack = 1 → 发 NACK（最后一个字节，结束读取）
 *============================================================================*/
void I2C_Ack(uint8_t ack)
{
    I2C_SCL_L();
    I2C_Delay();

    if (ack)
        I2C_SDA_H();        /* NACK: 释放 SDA */
    else
        I2C_SDA_L();        /* ACK:  拉低 SDA */

    I2C_Delay();
    I2C_SCL_H();
    I2C_Delay();
    I2C_SCL_L();
}
