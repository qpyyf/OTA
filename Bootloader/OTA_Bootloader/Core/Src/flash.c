#include "flash.h"
#include "main.h"
#include "stm32f4xx_it.h"


#define SRAM_size  512
#define DATA_Adress 0x08060000

uint16_t SRAM[SRAM_size];


uint32_t MyFlash_ReadWord(uint32_t DataAdress)
{
	return *((__IO uint32_t *)(DataAdress));
}

uint16_t MyFlash_ReadHalfWord(uint32_t DataAdress)
{
	return *((__IO uint16_t *)(DataAdress));
}

uint8_t MyFlash_ReadBite(uint32_t DataAdress)
{
	return *((__IO uint8_t *)(DataAdress));
}


/*
参数一：从扇区几开始擦除
参数二：总共要擦除几个扇区
参数三：置一全擦除，置零不全擦除
*/
// int8_t MyFLASH_Erase(int8_t Sectors,int8_t NbSectors,int8_t Allclear)
// {
// 	uint32_t SectorError=0;
//
// 	FLASH_EraseInitTypeDef FLASH_EraseInitStruct;
// 	if(Allclear==1){
// 	FLASH_EraseInitStruct.TypeErase=FLASH_TYPEERASE_MASSERASE;
// 	}else{
// 	FLASH_EraseInitStruct.TypeErase=FLASH_TYPEERASE_SECTORS;
// 	FLASH_EraseInitStruct.Sector=sec[Sectors];
// 	FLASH_EraseInitStruct.NbSectors=NbSectors;
// 	}
// 	FLASH_EraseInitStruct.VoltageRange=FLASH_VOLTAGE_RANGE_3;
// 	HAL_FLASH_Unlock();
// 	if(HAL_FLASHEx_Erase(&FLASH_EraseInitStruct,&SectorError)!=HAL_OK){
// 		return SectorError;
// 	}
// 	HAL_FLASH_Lock();
// 	return 1;
// }
int8_t MyFLASH_Erase(uint8_t Sector, uint8_t NbSectors, uint8_t AllClear)
{
	int8_t sec[]={FLASH_SECTOR_0,FLASH_SECTOR_1,FLASH_SECTOR_2,FLASH_SECTOR_3,FLASH_SECTOR_4,FLASH_SECTOR_5,FLASH_SECTOR_6,FLASH_SECTOR_7,
	FLASH_SECTOR_8,FLASH_SECTOR_9,FLASH_SECTOR_10,FLASH_SECTOR_11};
	if (Sector > 11 || NbSectors == 0 || (Sector + NbSectors) > 12)
		return -1;
	FLASH_EraseInitTypeDef EraseInit = {0};
	if (AllClear) {
		EraseInit.TypeErase = FLASH_TYPEERASE_MASSERASE;
	} else {
		EraseInit.TypeErase    = FLASH_TYPEERASE_SECTORS;
		EraseInit.Sector       = sec[Sector];
		EraseInit.NbSectors    = NbSectors;
	}
	EraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

	uint32_t SectorError = 0;

	__disable_irq();
	HAL_FLASH_Unlock();
	HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&EraseInit, &SectorError);
	HAL_FLASH_Lock();
	__enable_irq();

	return (status == HAL_OK) ? 0 : (int8_t)SectorError;
}

uint8_t MyFlash_Init(void)
{
	if(MyFlash_ReadHalfWord(DATA_Adress)!=0x5a5a){
		HAL_FLASH_Unlock();
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,DATA_Adress,0x5a5a);
		for(int i=1;i<512;i++)
		{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,DATA_Adress+(i*2),0x00);
		HAL_FLASH_Lock();
		}
	}
	for(int i=0;i<512;i++)
	{
	SRAM[i]=MyFlash_ReadHalfWord(DATA_Adress+(i*2));
	}
	return 1;
}

// uint8_t MyFlash_Save(void)
// {
// 	MyFLASH_Erase(7,1,0);///************/
//
// 	for(int i=0;i<512;i++)
// 	{
// 	HAL_FLASH_Unlock();
// 	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,DATA_Adress+(i*2),SRAM[i]);
// 	HAL_FLASH_Lock();
// 	}
// 	return 1;
// }
// void MyFlash_Clear(void)
// {
// 	for(int i=1;i<512;i++)
// 	{
// 		SRAM[i]=0;
// 	}
// 	MyFlash_Save(DATA_Adress,SRAM,512);
// }

uint8_t MyFlash_Save(uint32_t DataAdress,char *Buf ,uint32_t msglen)
{
	//MyFLASH_Erase(7,1,0);///************/
	HAL_FLASH_Unlock();// 解锁FLASH
	for(int i=0;i<msglen;i++)
	{// 保存msglen个字节一次写入一个字节
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,DataAdress+i,Buf[i]);
	}
	HAL_FLASH_Lock();// 锁定FLASH
	return 1;
}

