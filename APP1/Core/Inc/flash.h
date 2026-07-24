#ifndef __flash_h__
#define __flash_h__

#include "stm32f4xx_it.h"

extern uint8_t SRAM[];
uint8_t MyFlash_Erase(uint32_t Sectors,uint32_t NbBytes);
uint8_t MyFlash_Save(uint32_t DataAdress,char *Buf ,uint32_t msglen);
int8_t MyFLASH_Erase(int8_t Sectors,int8_t NbSectors,int8_t Allclear);

uint32_t MyFlash_ReadWord(uint32_t DataAdress);
uint16_t MyFlash_ReadHalfWord(uint32_t DataAdress);
uint8_t MyFlash_ReadBite(uint32_t DataAdress);
uint8_t MyFlash_ReadBite(uint32_t DataAdress);

uint8_t MyFlash_Init(void);
void MyFlash_Clear(void);

#endif

