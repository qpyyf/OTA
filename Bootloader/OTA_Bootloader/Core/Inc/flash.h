#ifndef __flash_h__
#define __flash_h__
#include "main.h"
#include "stm32f4xx_it.h"

extern uint16_t SRAM[];
uint32_t MyFlash_ReadWord(uint32_t DataAdress);
uint16_t MyFlash_ReadHalfWord(uint32_t DataAdress);
uint8_t MyFlash_ReadBite(uint32_t DataAdress);
// int8_t MyFLASH_Erase(int8_t Sectors,int8_t NbSectors,int8_t Allclear);
int8_t MyFLASH_Erase(uint8_t Sector, uint8_t NbSectors, uint8_t AllClear);
uint8_t MyFlash_Init(void);
// uint8_t MyFlash_Save(void);
void MyFlash_Clear(void);
uint8_t MyFlash_Save(uint32_t DataAdress,char *Buf ,uint32_t msglen);
#endif

