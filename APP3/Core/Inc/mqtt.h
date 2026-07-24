//
// Created by Lenovo on 2026/7/2.
//

#ifndef _MQTT_H_
#define _MQTT_H_
#include "fifo.h"
// ?????1  , ???WIFI
// ?????0  , ???4G
#define  MQTT_WIFI_4G_ENABLE    0
#define APP2_Flash_Bank 6
#define APP1_Flash_Bank 2
#define Application_2_Addr 0x08040000U
typedef struct
{
    char fw_title[32];
    char fw_version[32];
    uint32_t fw_size;
    char fw_checksum[32];
    uint8_t recv_buf[NSize];
    __IO uint8_t recv_flag  ; // OTA ���յ��̼����ݱ�ʶλ
    uint32_t request_id  ; // ��������ݰ�id��
    uint32_t chunk_id  ;   // ��������ݿ�id��
    uint32_t request_bytes  ; // �������ص��ֽ���
    uint32_t received_bytes ; // ���յ����ֽ����� ������ʾ����

}OTA_FW_Info_T ;

extern OTA_FW_Info_T OTA_Info; // ����OTA�̼���Ϣ�Ľṹ��
void MQTT_SendseniorData(void );
void MQTT_Connect_Server(void);
uint8_t ESP8266_Connect_MQTTServer(void);
uint8_t * Get_Cpu_ID(void);
uint8_t MQTT_Parse_JsonData(uint8_t *json);
uint8_t ML307_Connect_MQTTServer(void);
extern __IO uint8_t MQTTUpLoad_flag;
extern __IO uint8_t MQTTDownLoad_Flag;
extern __IO uint8_t MQTT_OTA_Flag;//������������ͷְ���ʶλ
uint8_t MQTT_Parse_DeviceData(uint8_t *json,uint32_t dataNum);
uint8_t MQTT_SendData(uint8_t *topic,uint32_t dataNum,char *status);
uint8_t CAT1_Parse_ATComander(uint8_t *recvbuf);
uint8_t MQTT_Parse_OTAData(uint8_t *json);
uint8_t MQTT_OTA_GetFW(void);
void Update_Progress(uint32_t len);
#endif //_MQTT_H_

