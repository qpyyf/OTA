//
// Created by Lenovo on 2026/6/28.
//
#include "modbus.h"
#include <stdio.h>
#include "main.h"
#include "GPIO.h"
#include "aht20.h"
#include "ina226.h"
#include "adc.h"
#include "mb.h"
//定义一个500ms的定时器用于modbus通信定时器到时刷新硬件状态
/**
 * @brief  定时器3超时标志位
 */
__IO uint8_t TIM3_Timerout_Flag = 0;
uint8_t SlavemodbusAddress = 0x01;
__IO uint8_t Modify_SlavemodbusAddress_Flag = 0;//修改从机地址标志位

/**
 * @brief  向从机地址发送数据单次一字节
 * @param Address 数据要写到at24c02内的地址
 * @param pdata 要写到at24c02的数据取指针传入
 */
uint8_t Modbus_WriteSlaveAddress(uint8_t Address,uint8_t * pdata) {
    if (HAL_I2C_Mem_Write(&hi2c1, AT24C02_ADDRESS_WRITE,Address,I2C_MEMADD_SIZE_8BIT,pdata, 1,1000)==HAL_OK)
        {return SET;  }
    else
        {return RESET;}
}

/**
 * @brief  从从机地址读取数据单次一字节
 * @param Address 数据要读取at24c02内的地址
 * @param pdata 要读取at24c02的数据取指针传入
 */
uint8_t ModBus_ReadAddress(uint8_t Address,uint8_t * pdata) {
    HAL_StatusTypeDef temp=HAL_I2C_Mem_Read(&hi2c1, AT24C02_ADDRESS_READ,Address,I2C_MEMADD_SIZE_8BIT,pdata,1,1000);
    if (* pdata==0x00) {
        * pdata=1;
    }
    if (temp==HAL_OK) {
        return SET;
    }
    else {
        return RESET;
    }
}

void Modbus_Init(void) {
    ModBus_ReadAddress(0,&SlavemodbusAddress);//读出是0函数自动处理为1
    printf("modbusAddress =%03d\n",SlavemodbusAddress);//打印从机地址 03表示前补0有3位显示
    eMBInit(MB_RTU,SlavemodbusAddress,0,115200,MB_PAR_NONE);
    eMBEnable();
}



void Modbus_parse(void) {
    if (TIM3_Timerout_Flag==1) {
        TIM3_Timerout_Flag = 0;
        AHT20_Read();//读取湿度温度
        INA226_Read();//读取电流电压功率
        ADC_CPU_Read();//读取CPU温度
        if (Modify_SlavemodbusAddress_Flag==1) {
            Modify_SlavemodbusAddress_Flag=0;
            SlavemodbusAddress=REG_HOLD_BUF[9];//从保持寄存器 REG_HOLD_BUF[9] 中读取从机地址
            Modbus_WriteSlaveAddress(0,&SlavemodbusAddress);
            HAL_Delay(10);
            eMBDisable();
            eMBClose();
            Modbus_Init();
        }

        /*---------------------LED1---------------------------*/
        if (REG_HOLD_BUF[0]&LED1_CMD) {
            GPIO_Control(LED1,ON);
        }
        else {
            GPIO_Control(LED1,OFF);//led1(pb8)
        }
        /*---------------------LED2---------------------------*/
        if (REG_HOLD_BUF[0]&LED2_CMD) {
            GPIO_Control(LED2,ON);
        }
        else {
            GPIO_Control(LED2,OFF);//led2(pb9)
        }
        /*---------------------beep---------------------------*/
        if (REG_HOLD_BUF[0]&BEEP_CMD) {
            GPIO_Control(beep,ON);
        }
        else {
            GPIO_Control(beep,OFF);//beep(pb10)
        }
        /*---------------------jdq---------------------------*/
        if (REG_HOLD_BUF[0]&RELAY_CMD) {
            GPIO_Control(jdq,ON);
        }
        else {
            GPIO_Control(jdq,OFF);//jdq(pb11)
        }
    }
}


