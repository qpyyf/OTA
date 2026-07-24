#include "main.h"
#include "rtc.h"
#include "sd3078.h"
#include "i2c.h"
#include "stdio.h"

void SD3078_ChargeDisable(void);
uint8_t BCD2DEC(uint8_t bcd)
{
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}

uint8_t DEC2BCD(uint8_t dec)
{
    return ((dec / 10) << 4) | (dec % 10);
}


void sd3078_Writeable()
{
	uint8_t cmd1[] = {0x10, 0x80};
	uint8_t cmd2[] = {0x0F, 0x84};
	HAL_I2C_Master_Transmit(&hi2c1, SD3078_ADDR_WRITE, cmd1, 2, 0xff);
	HAL_I2C_Master_Transmit(&hi2c1, SD3078_ADDR_WRITE, cmd2, 2, 0xff);
}

void SD3078_WriteDisable(void)
{
	uint8_t cmd1[] = {0x0F, 0x00};
	uint8_t cmd2[] = {0x10, 0x00};
	HAL_I2C_Master_Transmit(&hi2c1, SD3078_ADDR_WRITE, cmd1, 2, 0xff);
	HAL_I2C_Master_Transmit(&hi2c1, SD3078_ADDR_WRITE, cmd2, 2, 0xff);
}

/* ==================== 充电控制 ==================== */

void SD3078_ChargeEnable(uint8_t current)
{
	uint8_t data = 0x80 | (current & 0x03);   // EN=1 + 电流档位
	sd3078_Writeable();
	HAL_I2C_Master_Transmit(&hi2c1, SD3078_ADDR_WRITE, (uint8_t[]){0x18, data}, 2, 0xff);
	SD3078_WriteDisable();
}

void SD3078_ChargeDisable(void)
{
	sd3078_Writeable();
	HAL_I2C_Master_Transmit(&hi2c1, SD3078_ADDR_WRITE, (uint8_t[]){0x18, 0x00}, 2, 0xff);
	SD3078_WriteDisable();
}

void SD3078_SetTime(RTC_Time_t *t)
{
	uint8_t	bkp1sddress=0x15,recivedata=0;
//	HAL_I2C_Master_Transmit(&hi2c1, SD3078_ADDR_WRITE,&bkp1sddress , 1, 0xff);
//	HAL_I2C_Master_Receive(&hi2c1, SD3078_ADDR_READ,&recivedata , 1, 0xff);
	HAL_I2C_Mem_Read(&hi2c1, SD3078_ADDR_WRITE, 0x2C, I2C_MEMADD_SIZE_8BIT, &recivedata, 1, 0xff);//yong mem read bu ran hui you stop wei 
								//bu neng yong  HAL_I2C_Master_Transmit HAL_I2C_Master_Receive
	if(recivedata!=0x5A){
	printf(" %d \n",recivedata);
	uint8_t buf[8];
	buf[0] = 0x00;
	buf[1] = DEC2BCD(t->sec);
	buf[2] = DEC2BCD(t->min);
	buf[3] = DEC2BCD(t->hour) | 0x80;
	buf[4] = DEC2BCD(t->week);
	buf[5] = DEC2BCD(t->day);
	buf[6] = DEC2BCD(t->month);
	buf[7] = DEC2BCD(t->year);
	sd3078_Writeable();
	HAL_I2C_Master_Transmit(&hi2c1, SD3078_ADDR_WRITE, buf, 8, 0xff);
	uint8_t bkp[] = {0x2C, 0x5A};
	HAL_I2C_Master_Transmit(&hi2c1, SD3078_ADDR_WRITE, bkp, 2, 0xff);
	SD3078_WriteDisable();	
	SD3078_ChargeDisable();
	}
}

void SD3078_GetTime(RTC_Time_t *t)
{
	uint8_t reg = 0x00;
	uint8_t tim[7] = {0};
	HAL_I2C_Mem_Read(&hi2c1, SD3078_ADDR_WRITE, reg, I2C_MEMADD_SIZE_8BIT, tim, 7, 0xff);
	t->sec   = BCD2DEC(tim[0] & 0x7F);
	t->min   = BCD2DEC(tim[1]);
	t->hour  = BCD2DEC(tim[2] & 0x7F);
	t->week  = BCD2DEC(tim[3]);
	t->day   = BCD2DEC(tim[4]);
	t->month = BCD2DEC(tim[5]);
	t->year  = BCD2DEC(tim[6]);
}
