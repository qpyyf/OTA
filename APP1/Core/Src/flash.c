#include "flash.h"

#include "mqtt.h"
#include "stm32f4xx_it.h"


#define SRAM_size  256
#define DATA_Adress 0x08060000

uint8_t SRAM[SRAM_size];


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
����һ������������ʼ����
���������ܹ�Ҫ������������
����������һȫ���������㲻ȫ����
*/
 int8_t MyFLASH_Erase(int8_t Sectors,int8_t NbSectors,int8_t Allclear)
{
	uint32_t SectorError=0;
	int8_t sec[]={FLASH_SECTOR_0,FLASH_SECTOR_1,FLASH_SECTOR_2,FLASH_SECTOR_3,FLASH_SECTOR_4,FLASH_SECTOR_5,FLASH_SECTOR_6,FLASH_SECTOR_7,
	FLASH_SECTOR_8,FLASH_SECTOR_9,FLASH_SECTOR_10,FLASH_SECTOR_11};
	FLASH_EraseInitTypeDef FLASH_EraseInitStruct;
	if(Allclear==1){
	FLASH_EraseInitStruct.TypeErase=FLASH_TYPEERASE_MASSERASE;
	}else{
	FLASH_EraseInitStruct.TypeErase=FLASH_TYPEERASE_SECTORS;
	FLASH_EraseInitStruct.Sector=sec[Sectors];
	FLASH_EraseInitStruct.NbSectors=NbSectors;
	}
	FLASH_EraseInitStruct.VoltageRange=FLASH_VOLTAGE_RANGE_3;
	HAL_FLASH_Unlock();
	if(HAL_FLASHEx_Erase(&FLASH_EraseInitStruct,&SectorError)!=HAL_OK){
		return SectorError;
	}
	HAL_FLASH_Lock();
	return 1;
}

// uint8_t MyFlash_Init(void)
// {
// 	if(MyFlash_ReadHalfWord(DATA_Adress)!=0x5a5a){
// 		HAL_FLASH_Unlock();
// 		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,DATA_Adress,0x5a5a);
// 		for(int i=1;i<256;i++)
// 		{
// 		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,DATA_Adress+(i*2),0x00);
// 		HAL_FLASH_Lock();
// 		}
// 	}
// 	for(int i=0;i<512;i++)
// 	{
// 	SRAM[i]=MyFlash_ReadHalfWord(DATA_Adress+(i*2));
// 	}
// 	return 1;
// }
/*
 * ��������
 * @param Sectors ��ʼ������ַ��
 * @param NbBytes ������ֽ������Զ�������Ҫ������������
 * @return uint8_t 0 �ɹ�������ֵ ʧ��
 */
uint8_t MyFlash_Erase(uint32_t Sectors,uint32_t NbBytes) {
	(void)NbBytes;  // 不使用NbBytes, 始终擦除整个APP区域以重置标志位
	/*************************** APP1区域擦除(扇区2-5, 固定4个扇区) *******************************/
	if (Sectors==APP1_Flash_Bank) {
		MyFLASH_Erase(Sectors,4,0);
		return 1;
	}
	else if (Sectors==APP2_Flash_Bank) {
		MyFLASH_Erase(Sectors,2,0);  // 必须擦除扇区7, 因为标志位在扇区7末尾
		return 1;
	}
	return 0;
}
/*
 * ���溯��
 * @param DataAdress �����ַ
 *  @param Buf �������ݻ�����
 * @param msglen �������ݳ���
 * @return uint8_t 0 �ɹ�������ֵ ʧ��
 */
//(uint32_t *)OTA_Info.recv_buf, msglen
uint8_t MyFlash_Save(uint32_t DataAdress,char *Buf ,uint32_t msglen)
{
	//MyFLASH_Erase(7,1,0);///************/
	HAL_FLASH_Unlock();// ����FLASH
	for(int i=0;i<msglen;i++)
	{// ����msglen���ֽ�һ��д��һ���ֽ�
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,DataAdress+i,Buf[i+0]);
	}
	HAL_FLASH_Lock();// ����FLASH
	return 1;
}
// void MyFlash_Clear(void)
// {
// 	for(int i=1;i<512;i++)
// 	{
// 		SRAM[i]=0;
// 	}
// 	MyFlash_Save();
// }



