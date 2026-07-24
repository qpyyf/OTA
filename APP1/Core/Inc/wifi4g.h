#ifndef __WIFI4G_H_
#define __WIFI4G_H_
#include <stdint.h>
#include "main.h"
#include "fifo.h"

enum
{
  WIFI4G_NOT = 0, // Not found
  WIFI4G_OK,
  WIFI4G_ERROR
};
// 下行主题1   负责接收服务器的命令
#define  SUB1  "v1/devices/me/rpc/request/+"
// 下行主题2   负责接收服务器的OTA信息数据
#define  SUB2  "v1/devices/me/attributes"
// 下行主题3   传输OTA的文件的内容     AT+MQTTPUB=0,"v2/fw/request/143488/chunk/0",1,0,0,3,"256"\r\n
#define  SUB3  "v2/fw/response/+/chunk/#"
// 上行主题1   上传服务器数据的
#define  PUB1  "v1/devices/me/telemetry"
// 上行主题2   回复服务器下发的命令
#define  PUB2  "v1/devices/me/rpc/response/"
// 上行主题3    请求服务器下发数据快
#define  PUB3  "v2/fw/request/+/chunk/#"
#define Application_Size 0x38000
extern __IO uint8_t WIFI4G_TouChuan_Flag; // 透传标志
extern __IO uint8_t WIFI4G_CMD_Status; // OK状态标志位

extern uint8_t Parse_Substr[];

uint8_t Test_WIFI4G_CMD_Status(uint32_t Timeout);
uint8_t WIFI4G_Parse_Queue(sequeue_t *sq);
uint8_t ESP8266_Connect_WIFI(void);
void ESP8266_Init(void);
void ESP8266_Test(void);
uint8_t ESP8266_Connect_TCPServer(void);
uint8_t MQTT_Connect_MQTTServer(void);
#endif
