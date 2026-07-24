#include "ina226.h"
#include <stdio.h>
#include "MODBUS.H"
/**
 * @brief    INA226初始化
 * @param  	 无
 * @retval   无
 */
void INA226_Init(void)
{
    uint8_t tData[3];
    tData[0] = Configuration_Register;
    tData[1] = Configuration_Register_Init >> 8;
    tData[2] = (uint8_t)Configuration_Register_Init;
    HAL_I2C_Master_Transmit(&hi2c1, INA226_ADDR, tData, 3, 0xff);
    HAL_Delay(5);
    tData[0] = Calibration_Register;
    tData[1] = Calibration_Register_Init >> 8;
    tData[2] = (uint8_t)Calibration_Register_Init;
    HAL_I2C_Master_Transmit(&hi2c1, INA226_ADDR, tData, 3, 0xff);
}

/**
 * @brief    INA226读取总线电压值
 * @param  	 无
 * @retval   总线电压值
 */
uint16_t INA226_Read_Bus_Voltage(void)
{
    uint16_t Bus_Voltage;
    uint8_t rData[2];
    uint8_t tData[1] = {Bus_Voltage_Register};
    HAL_I2C_Master_Transmit(&hi2c1, INA226_ADDR, tData, 1, 0xff);
    HAL_I2C_Master_Receive(&hi2c1, INA226_ADDR, rData, 2, 0xff);
    Bus_Voltage = rData[0] << 8 | rData[1];
    return Bus_Voltage;
}

/**
 * @brief    INA226读取电流
 * @param  	 无
 * @retval   总线电流值
 */
uint16_t INA226_Read_Current(void)
{
    uint16_t Current;
    uint8_t rData[2];
    uint8_t tData[1] = {Current_Register};
    HAL_I2C_Master_Transmit(&hi2c1, INA226_ADDR, tData, 1, 0xff);
    HAL_I2C_Master_Receive(&hi2c1, INA226_ADDR, rData, 2, 0xff);
    Current = rData[0] << 8 | rData[1];
    return Current;
}

/**
 * @brief    INA226读取功率
 * @param  	 无
 * @retval   功率值
 */
uint16_t INA226_Read_Pow(void)
{
    uint16_t Pow;
    uint8_t rData[2];
    uint8_t tData[1] = {Power_Register};
    HAL_I2C_Master_Transmit(&hi2c1, INA226_ADDR, tData, 1, 0xff);
    HAL_I2C_Master_Receive(&hi2c1, INA226_ADDR, rData, 2, 0xff);
    Pow = rData[0] << 8 | rData[1];
    return Pow;
}

/**
 * @brief    INA226读取电流、电压、功率值
 * @param  	 无
 * @retval   无
 */
void INA226_Read(void)
{
		//printf("INA226 Test\n")
		//uint8_t str[64];
		// OLED_ShowStr(16,0, (unsigned char*)"INA226 Test", 2);
		uint16_t Current_Original = INA226_Read_Current();   // 读取电流
		uint16_t Voltage_Original = INA226_Read_Bus_Voltage(); // 读取电压
		uint16_t Pow_Original     = INA226_Read_Pow(); // 读取功率
		/*-------------------------  转换为实际值 -----------------------------------*/
		float Current = Current_Register_LSB * Current_Original;//乘以0.1，得到mA单位
		float Voltage = (Bus_Voltage_Register_LSB * Voltage_Original) / 1000.0;//乘以1.25，得到V单位
		float Pow     = Power_Register_LSB * Pow_Original;//乘以0.1*25，得到mW单位

		REG_HOLD_BUF[3] =(uint16_t)(Voltage*100);//V乘以100便于modbus传输
		REG_HOLD_BUF[4] =(uint16_t)Current;//mA
		REG_HOLD_BUF[5] =(uint16_t)Pow;//mW
		// printf("Current:%.0f ,%.3f ,%.0f \n",Current,Voltage,Pow);


//		sprintf((char *)str,"Current:%.0f mA",Current);
//		OLED_ShowStr(0,2, (unsigned char*)str, 2);
//		sprintf((char *)str,"Voltage:%.3f V",Voltage);
//		OLED_ShowStr(0,4, (unsigned char*)str, 2);
//		sprintf((char *)str,"Pow    :%.0f mW",Pow);
//		OLED_ShowStr(0,6, (unsigned char*)str, 2);
//		 HAL_Delay(1000);
}
