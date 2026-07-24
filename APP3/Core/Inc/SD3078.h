#ifndef __SD3078_h__
#define __SD3078_h__
#include "main.h"

#define SD3078_ADDR_WRITE  0x64   // 7λд��ַ

/* 充电电流选择 */
#define SD3078_CHARGE_10K  0x00   // 10kΩ 限流
#define SD3078_CHARGE_5K   0x01   // 5kΩ 限流（推荐，~80μA）
#define SD3078_CHARGE_2K   0x02   // 2kΩ 限流
#define SD3078_CHARGE_OFF  0x03   // 禁止充电
 typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t week;
    uint8_t day;
    uint8_t month;
    uint8_t year;    // ����λ���� 26 = 2026 ��
} RTC_Time_t;



void SD3078_SetTime(RTC_Time_t *t);
void SD3078_GetTime(RTC_Time_t *t);
void SD3078_ChargeEnable(uint8_t current);
void SD3078_ChargeDisable(void);
#endif
