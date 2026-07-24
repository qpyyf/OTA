//
// Created by Lenovo on 2026/6/28.
//
#include "main.h"
#ifndef _MODBUS_H_
#define _MODBUS_H_

#define LED1_CMD  (0x1<<0)
#define LED2_CMD  (0x1<<1)
#define BEEP_CMD  (0x1<<2)
#define RELAY_CMD (0x1<<3)

extern __IO uint8_t Modify_SlavemodbusAddress_Flag;//修改从机地址标志位
extern __IO uint8_t TIM3_Timerout_Flag;
extern uint16_t REG_HOLD_BUF[];
extern uint8_t SlavemodbusAddress;

void Modbus_parse(void);
void Modbus_Init(void);
uint8_t Modbus_WriteSlaveAddress(uint8_t Address,uint8_t * pdata);
uint8_t ModBus_ReadAddress(uint8_t Address,uint8_t * pdata);




#endif //INC_01_FREEMODBUS_MODBUS_H
