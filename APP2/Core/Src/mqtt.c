//
// Created by Lenovo on 2026/7/2.
//
#include "main.h"
#include "mqtt.h"
//#include "main.h"
#include "wifi4g.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include "flash.h"
#include "modbus.h"
#include  "mycrc.h"
#include  "iwdg.h"
#include "FreeRTOS_Demo.h"
#define UART huart2
__IO uint8_t MQTT_OTA_Flag=0;//������������ͷְ���ʶλ
__IO uint8_t MQTTUpLoad_flag=0;//��ʼ�����ϴ����ݣ�5��һ��
__IO uint8_t MQTTDownLoad_Flag=0;//��ʼ������������ñ�־λ��������ʼ�����ı�־λ��
uint8_t * Get_Cpu_ID(void) {
    static uint8_t CPUIDbuf[33]={0};
    uint32_t ID_buf[3]={0};
    ID_buf[0]= * (__IO uint32_t *)0x1FFF7A10;
    ID_buf[1]= * (__IO uint32_t *)0x1FFF7A14;
    ID_buf[2]= * (__IO uint32_t *)0x1FFF7A18;
    snprintf((char * )CPUIDbuf,sizeof(CPUIDbuf),"%08x%08x%08x",(unsigned int)ID_buf[0],(unsigned int)ID_buf[1],(unsigned int)ID_buf[2]);
    return CPUIDbuf;
}

/**
 * @brief  ����broker.emqx.io mqtt������������
 * @retval SET ���ӳɹ�
 * @retval RESET ����ʧ��
 */
void  MQTT_Connect_Server(void) {
    #if MQTT_WIFI_4G_ENABLE
    while (ESP8266_Connect_WIFI() != SET) {//wifiҪ����wifi������mqtt������
    }
    while (ESP8266_Connect_MQTTServer() != SET) {
    }
    #else
    while (ML307_Connect_MQTTServer() != SET) {//4g����ֱ������mqtt������
    }
    #endif
}

uint8_t ESP8266_Connect_MQTTServer(void)
{
    uint8_t buf[100] = {0};
    printf("CPU_ID:%s\n",(char * )Get_Cpu_ID());
    WIFI4G_CMD_Status = WIFI4G_NOT;			// ��ʼ����־λ
    snprintf((char *)buf,sizeof(buf), "AT+MQTTUSERCFG=0,1,\"%s\",\"\",\"\",0,0,\"\"\r\n",(char * )Get_Cpu_ID()); // ������
    printf("buf=%s\n",(char * )buf);
    HAL_UART_Transmit(&UART, buf, strlen((char *)buf), 1000);

    // OLED_ShowStr(0, 4, (unsigned char *)"AT+CIPMUX", 2); // ����8*16�ַ�
    if (Test_WIFI4G_CMD_Status(1000) == WIFI4G_ERROR)
    {
        // OLED_ShowStr(0, 4, (unsigned char *)"TouChuan...   --", 2); // ����8*16�ַ�
        return RESET;												// �ȴ�OK����
    }
    // ����mqtt������
    WIFI4G_CMD_Status = WIFI4G_NOT;			// ��ʼ����־λ
    strcpy((char *)buf, "AT+MQTTCONN=0,\"broker.emqx.io\",1883,1\r\n"); // ������
    printf("buf=%s\n",(char * )buf);
    HAL_UART_Transmit(&UART, buf, strlen((char *)buf), 1000);

    // OLED_ShowStr(0, 4, (unsigned char *)"AT+CIPMUX", 2); // ����8*16�ַ�
    if(Test_WIFI4G_CMD_Status(5*1000) == WIFI4G_ERROR)  {
        //OLED_ShowStr(0,4,(unsigned char *)"MQTTServer... --",2);    //����8*16�ַ�
        return RESET ;
    }

    //  ����������Ϣ , ����һ���������ݵ�����
    WIFI4G_CMD_Status = WIFI4G_NOT ;     // ��ʼ����־λ
    //OLED_ShowStr(0,4,(unsigned char *)"AT+MQTTSUB",2);              //����8*16�ַ�
    sprintf((char *)buf,"AT+MQTTSUB=0,\"ESP8266\",0\r\n");
    HAL_UART_Transmit(&UART, buf,strlen((char *)buf),1000);
    uint8_t ret = Test_WIFI4G_CMD_Status(5*1000) ;
    if (ret == WIFI4G_OK)
    {
        printf("MQTTServerConnect... OK\n");
        printf("���������ӳɹ� \n");
        MQTTDownLoad_Flag=1;
        //OLED_ShowStr(0,4,(unsigned char *)"MQTTServer... OK",2);    //����8*16�ַ�
    }
    else
    {
        printf("MQTTServerConnect... FAILED\n");
        printf("����������ʧ�� \n");
        //OLED_ShowStr(0,4,(unsigned char *)"MQTTServer... --",2);    //����8*16�ַ�
        return  RESET;
    }
    return SET;
}
/*
 * @brief  ����broker.emqx.io mqtt������������
 * @param  None
 * @retval SET ���ӳɹ�
 * @retval RESET ����ʧ��
 */
uint8_t ML307_Connect_MQTTServer(void)
{
    uint8_t buf[100] = {0};
    strcpy((char *)Parse_Substr, "OK\r\n");
    // ����4gģ��
    // snprintf((char *)buf,sizeof(buf), "AT+REST\r\n");
    // printf("buf=%s\n",(char * )buf);
    // HAL_UART_Transmit(&UART, buf, strlen((char *)buf), 1000);
    // HAL_Delay(300);
    /*----------------------����MQTT����------------------------*/
    // ���������ϴ�����DTUTASK��1��ʾ������20��ʾMQTT����
    //  AT+DTUTASK="1","20"//����MQTT����
    WIFI4G_CMD_Status = WIFI4G_NOT;			// ��ʼ����־λ
    snprintf((char *)buf,sizeof(buf), "AT+DTUTASK=\"1\",\"20\"\r\n"); //��ʼ��4g
    printf("buf=%s\n",(char * )buf);
    HAL_UART_Transmit(&UART, buf, strlen((char *)buf), 1000);
    if (Test_WIFI4G_CMD_Status(1000) == WIFI4G_ERROR)
    {
        // OLED_ShowStr(0, 4, (unsigned char *)"TouChuan...   --", 2); // ����8*16�ַ�
        return RESET;												// �ȴ�OK����
    }

    printf("CPU_ID:%s\n",(char * )Get_Cpu_ID());
    /*----------------------����MQTT�ͻ���ID------------------------*/
    //AT+MQTT=""//
    WIFI4G_CMD_Status = WIFI4G_NOT;			// ��ʼ����־λ
    snprintf((char *)buf,sizeof(buf), "AT+MQTT=\"%s\"\r\n",(char * )Get_Cpu_ID()); //��ʼ��4g
    printf("buf=%s\n",(char * )buf);
    HAL_UART_Transmit(&UART, buf, strlen((char *)buf), 1000);
    // OLED_ShowStr(0, 4, (unsigned char *)"AT+CIPMUX", 2); // ����8*16�ַ�
    if (Test_WIFI4G_CMD_Status(1000) == WIFI4G_ERROR)
    {
        // OLED_ShowStr(0, 4, (unsigned char *)"TouChuan...   --", 2); // ����8*16�ַ�
        return RESET;												// �ȴ�OK����
    }
    /*----------------------����mqtt������------------------------*/
    // ����mqtt������
    //  AT+MQTTIP="broker.emqx.io","1883"//����mqtt��������ַ�Ͷ˿�
    WIFI4G_CMD_Status = WIFI4G_NOT;			// ��ʼ����־λ
    strcpy((char *)buf, "AT+MQTTIP=\"broker.emqx.io\",\"1883\"\r\n"); // ������
    printf("buf=%s\n",(char * )buf);
    HAL_UART_Transmit(&UART, buf, strlen((char *)buf), 1000);
    // OLED_ShowStr(0, 4, (unsigned char *)"AT+CIPMUX", 2); // ����8*16�ַ�
    if(Test_WIFI4G_CMD_Status(5*1000) == WIFI4G_ERROR)  {
        //OLED_ShowStr(0,4,(unsigned char *)"MQTTServer... --",2);    //����8*16�ַ�
        return RESET ;
    }
    /*----------------------����һ����������------------------------*/
    WIFI4G_CMD_Status = WIFI4G_NOT ;     // ��ʼ����־λ
    sprintf((char *)buf,"AT+MQPUB=\"1\",\"1\",\"4\",\"STM32/Upload/F407/%s\"\r\n",(char * )Get_Cpu_ID());//��������
    HAL_UART_Transmit(&UART, buf,strlen((char *)buf),1000);
    if(Test_WIFI4G_CMD_Status(5*1000) == WIFI4G_ERROR)  {
        //OLED_ShowStr(0,4,(unsigned char *)"MQTTServer... --",2);    //����8*16�ַ�
        return RESET ;
    }
    /*----------------------����һ����������------------------------*/
    //  ����������Ϣ , ����һ����������
    //  AT+MQSUB="1","1","4","STM32/Download/F407/%s"//��������
    WIFI4G_CMD_Status = WIFI4G_NOT ;     // ��ʼ����־λ
    sprintf((char *)buf,"AT+MQSUB=\"1\",\"1\",\"4\",\"STM32/Download/F407/%s\"\r\n",(char * )Get_Cpu_ID());
    HAL_UART_Transmit(&UART, buf,strlen((char *)buf),1000);
    if(Test_WIFI4G_CMD_Status(5*1000) == WIFI4G_ERROR)  {
        //OLED_ShowStr(0,4,(unsigned char *)"MQTTServer... --",2);    //����8*16�ַ�
        return RESET ;
    }
    // WIFI4G_CMD_Status = WIFI4G_NOT ;     // ��ʼ����־λ
    // sprintf((char *)buf,"AT+MQTTSUB=0,\"STM32/Download/F407/%s\",0\r\n",(char * )Get_Cpu_ID());
    // HAL_UART_Transmit(&UART, buf,strlen((char *)buf),1000);
    /*----------------------����AT+MQMULTEN=0���õ�ͨ��ͨ��------------------------*/
    WIFI4G_CMD_Status = WIFI4G_NOT ;     // ��ʼ����־λ
    sprintf((char *)buf,"AT+MQMULTEN=0\r\n");
    HAL_UART_Transmit(&UART, buf,strlen((char *)buf),1000);
    /*----------------------����4gģ��------------------------*/
    //��������4gģ�����ò���Ч
    strcpy((char *)Parse_Substr, "MQTT_CONNECT:");//Ѱ��MQTT_CONNECT:�ַ�����ʾ������Ч���ӳɹ�
    WIFI4G_CMD_Status = WIFI4G_NOT ;     // ��ʼ����־λ
    snprintf((char *)buf,sizeof(buf), "AT+REST\r\n");
    HAL_UART_Transmit(&UART, buf, strlen((char *)buf), 1000);
    uint8_t ret = Test_WIFI4G_CMD_Status(180*1000) ;
    if (ret == WIFI4G_OK)
    {
        printf("MQTTServerConnect... OK\n");
        printf("���������ӳɹ� \n");
        MQTTDownLoad_Flag=1;
        //OLED_ShowStr(0,4,(unsigned char *)"MQTTServer... OK",2);    //����8*16�ַ�
    }
    else
    {
        printf("MQTTServerConnect... FAILED\n");
        printf("����������ʧ�� \n");
        //OLED_ShowStr(0,4,(unsigned char *)"MQTTServer... --",2);    //����8*16�ַ�
        return  RESET;
    }
    return SET;
}
/*
 * @brief  ���Ӻ���mqtt��������ͨ�������뷢������
 * @retval SET ���ӳɹ�
 * @retval RESET ����ʧ��
 */

uint8_t MQTT_Connect_MQTTServer(void)
{
     uint8_t ret;
            uint8_t buf[128]={0};
            /**************************---1---************************************/
            WIFI4G_CMD_Status = WIFI4G_NOT ;     // 初始化标志位
            strcpy((char *)Parse_Substr,"OK\r\n");   // 命令成功的返回值
            sprintf((char *)buf,"AT+MQMULTEN=1\r\n"); // 使能多通道订阅与发布主题
            HAL_UART_Transmit(&huart2,buf,strlen((char *)buf),1000);
            if(Test_WIFI4G_CMD_Status(1000) == WIFI4G_ERROR)  {
                    return RESET; }

            /**************************---2---***********************************/
            WIFI4G_CMD_Status = WIFI4G_NOT ;     // 初始化标志位
            strcpy((char *)Parse_Substr,"OK\r\n");  // 命令成功的返回值
            sprintf((char *)buf,"AT+MQTTFILTER=0\r\n");  //不过滤任何主题
            HAL_UART_Transmit(&huart2,buf,strlen((char *)buf),1000);
            if(Test_WIFI4G_CMD_Status(1000) == WIFI4G_ERROR)  {
                    return RESET; }

            /***************************---3---**********************************/
            WIFI4G_CMD_Status = WIFI4G_NOT ;     // 初始化标志位
            strcpy((char *)Parse_Substr,"OK\r\n");  // 命令成功的返回值
            // 订阅服务器 下行主题1
            sprintf((char *)buf,"AT+MQSUBM=0,1,0,4,\"%s\"\r\n",SUB1);
            HAL_UART_Transmit(&huart2,buf,strlen((char *)buf),1000);
            if(Test_WIFI4G_CMD_Status(1000) == WIFI4G_ERROR)  {
                    return RESET; }

            /***************************---4---**********************************/
            WIFI4G_CMD_Status = WIFI4G_NOT ;     // 初始化标志位
            strcpy((char *)Parse_Substr,"OK\r\n");  // 命令成功的返回值
            // 订阅服务器 下行主题2
            sprintf((char *)buf,"AT+MQSUBM=1,1,0,4,\"%s\"\r\n",SUB2);
            HAL_UART_Transmit(&huart2,buf,strlen((char *)buf),1000);
            if(Test_WIFI4G_CMD_Status(1000) == WIFI4G_ERROR)  {
                    return RESET; }

            /***************************---5---**********************************/
            WIFI4G_CMD_Status = WIFI4G_NOT ;     // 初始化标志位
            strcpy((char *)Parse_Substr,"OK\r\n");  // 命令成功的返回值
            // 订阅服务器 下行主题2
            sprintf((char *)buf,"AT+MQSUBM=2,1,0,4,\"%s\"\r\n",SUB3);
            HAL_UART_Transmit(&huart2,buf,strlen((char *)buf),1000);
            if(Test_WIFI4G_CMD_Status(1000) == WIFI4G_ERROR)  {
                    return RESET; }

            /***************************---6---**********************************/
            WIFI4G_CMD_Status = WIFI4G_NOT ;     // 初始化标志位
            strcpy((char *)Parse_Substr,"MQTT_CONNECT:");    // 服务器连接功的返回值
            // 复位4G模组, 复位后自动连接服务器
            sprintf((char *)buf,"AT+REST\r\n");
            HAL_UART_Transmit(&huart2,buf,strlen((char *)buf),1000);
            ret = Test_WIFI4G_CMD_Status(1000*180) ;  // 延时180秒
            if (ret == WIFI4G_OK) {
                //OLED_ShowStr(0,3,(unsigned char *)"MQTTServer... OK",2);    //测试8*16字符
                printf("MQTTServerConnect... OK \r\n");
            }
            else
            {
                //OLED_ShowStr(0,3,(unsigned char *)"MQTTServer... --",2);    //测试8*16字符
                printf("MQTTServerConnect... FAILED \r\n");
                return  RESET;
            }
    return SET;
}





// �������ݵ�mqtt������
/*----------------------�������ݵ�mqtt������------------------------*/
/*
 * 4G/wifi�������ݵ�������
 *
 */
void MQTT_SendseniorData(void ) {
    static uint8_t buf[256]={0};
    static uint8_t Send_buf[128]={0};
#if MQTT_WIFI_4G_ENABLE
    /*----------------------WIFI�������ݵ�mqtt������------------------------*/
    sprintf((char *)Send_buf,"{\\\"TP\\\":%d\\,\\\"RH\\\":%d\\,\\\"VO\\\":%d\\,\\\"CU\\\":%d\\,\\\"PW\\\":%d\\,\\\"CPU\\\":%d}",
     REG_HOLD_BUF[0],REG_HOLD_BUF[1],REG_HOLD_BUF[2],REG_HOLD_BUF[3],REG_HOLD_BUF[4],REG_HOLD_BUF[5]);
  //  printf("Send_buf=%s\n",Send_buf);
    snprintf((char *)buf,sizeof(buf),"AT+MQTTPUB=0,\"ESP8266\",\"%s\",0,0\r\n",(char * )Send_buf);
#else
    /*----------------------4G�������ݵ�mqtt������------------------------*/
//     AT
// +MQTTPUB=<connect_id>,<t
// opic>,<qos>,<retain>,<dup>,
// <msg_len>[,<message>]
    // MQPUB,1,PUB2,123
    #if 0
    sprintf((char *)Send_buf,"{\"TP\":%d,\"RH\":%d,\"VO\":%d,\"CU\":%d,\"PW\":%d,\"VR\":%d,\"CPU\":%d}",
    REG_HOLD_BUF[1],REG_HOLD_BUF[2], REG_HOLD_BUF[3],REG_HOLD_BUF[4],
    REG_HOLD_BUF[5],0,REG_HOLD_BUF[7]);
    #elif 1
    sprintf((char *)Send_buf,"{TP:%d,RH:%d,VO:%d,CU:%d,PW:%d,VR:%d,CPU:%d}",
    REG_HOLD_BUF[1],REG_HOLD_BUF[2], REG_HOLD_BUF[3],REG_HOLD_BUF[4],
    REG_HOLD_BUF[5],0,REG_HOLD_BUF[7]);
    #endif
    //printf("%s\n",Send_buf);
    // �������ݵ�mqtt������
    sprintf((char *)buf,"MQPUB,1,%s,%s",PUB1,(char *)Send_buf);
    printf("buf=%s\n",(char *)buf);
#endif
    /*----------------------�������ݵ�mqtt������------------------------*/
    HAL_UART_Transmit(&UART, buf, strlen((char *)buf), 1000);
}
// ��cjson����mqtt��������������
uint8_t MQTT_Parse_JsonData(uint8_t *json) {
    cJSON * cJSON_device=NULL;
    cJSON * CJSON_LED1=NULL;
    cJSON * CJSON_LED2=NULL;
    cJSON * CJSON_BEEP=NULL;
    cJSON * CJSON_RELAY=NULL;
    cJSON_device=cJSON_Parse((char *)json);
    if (cJSON_device==NULL) {
        printf("cJSON_Parse failed\n");
        return RESET;
    }
    CJSON_LED1=cJSON_GetObjectItem(cJSON_device,"LED1");
    CJSON_LED2=cJSON_GetObjectItem(cJSON_device,"LED2");
    CJSON_BEEP=cJSON_GetObjectItem(cJSON_device,"BEEP");
    CJSON_RELAY=cJSON_GetObjectItem(cJSON_device,"RELAY");
    if (CJSON_LED1!=NULL) {
        printf("LED1=%d\n",CJSON_LED1->valueint);
        if (CJSON_LED1->valueint ==0) {
            REG_HOLD_BUF[0]&=~LED1_CMD;//REG_HOLD_BUF[0]�ĵ�0λΪLED1�Ŀ���λд0
        }
        else {
            REG_HOLD_BUF[0]|=LED1_CMD;//REG_HOLD_BUF[0]�ĵ�0λΪLED1�Ŀ���λд1
        }
    }
    if (CJSON_LED2!=NULL) {
        printf("LED2=%d\n",CJSON_LED2->valueint);
        if (CJSON_LED2->valueint ==0) {
            REG_HOLD_BUF[0]&=~LED2_CMD;//REG_HOLD_BUF[0]�ĵ�1λΪLED2�Ŀ���λд0
        }
        else {
            REG_HOLD_BUF[0]|=LED2_CMD;//REG_HOLD_BUF[0]�ĵ�1λΪLED2�Ŀ���λд1
        }
    }
    if (CJSON_BEEP!=NULL) {
        printf("BEEP=%d\n",CJSON_BEEP->valueint);
        if (CJSON_BEEP->valueint ==0) {
            REG_HOLD_BUF[0]&=~BEEP_CMD;//REG_HOLD_BUF[0]�ĵ�2λΪBEEP�Ŀ���λд0
        }
        else {
            REG_HOLD_BUF[0]|=BEEP_CMD;//REG_HOLD_BUF[0]�ĵ�2λΪBEEP�Ŀ���λд1
        }
    }
    if (CJSON_RELAY!=NULL) {
        printf("BEEP=%d\n",CJSON_RELAY->valueint);
        if (CJSON_RELAY->valueint ==0) {
            REG_HOLD_BUF[0]&=~RELAY_CMD;//REG_HOLD_BUF[0]�ĵ�3λΪRELAY�Ŀ���λд0
        }
        else {
            REG_HOLD_BUF[0]|=RELAY_CMD;//REG_HOLD_BUF[0]�ĵ�3λΪRELAY�Ŀ���λд1
        }
    }

    cJSON_Delete(cJSON_device);//������ɺ�һ��Ҫ�ͷ��ڴ�
    return SET;
}
/*
 *4g��������������
 *
 */

OTA_FW_Info_T OTA_Info ; // ����OTA�̼���Ϣ�Ľṹ��
uint8_t CAT1_Parse_ATComander(uint8_t *recvbuf)
{
        char * leftp = (char *)recvbuf ;
        char * rightp = leftp ;
        char * startp = leftp ;
        uint32_t dataNum = 0 ;
        memset(OTA_Info.recv_buf,0,sizeof(OTA_Info.recv_buf)) ; // �������
        // �����������·�������
        char substr[64] = {"v1/devices/me/rpc/request/"};
        if( (startp = strstr((char *)startp,substr) ) != NULL )
        {
                dataNum = strtol(startp + strlen(substr), NULL, 10);//startp + strlen(substr)/request/��������ֲ���ָ��ָ��//10����
                leftp   = strstr((char *)startp,"{"); // �ҵ�������
                rightp  = strstr((char *)startp,"}"); // �ҵ��һ�����
                if( (leftp != NULL) && (rightp != NULL))// �ҵ������ź��һ�����
                {
                        strncpy((char *)OTA_Info.recv_buf,leftp,rightp-leftp+1); // �����ַ���cjson֧����Ҫ����{}{1616}
                        printf("jsonbuf=%s | dataNum=%d\n",OTA_Info.recv_buf,dataNum);
                        MQTT_Parse_DeviceData((uint8_t*)OTA_Info.recv_buf,dataNum);// ����json�ַ����������豸
                        return SET;
                }
                //startp = startp + strlen(substr); // �ƶ�λ��

        }
        /***********************************************************/
        // �����������·�������OTA attributes
        memset(substr,0,sizeof(substr));
        startp = (char *)recvbuf ;
        strcpy(substr,"v1/devices/me/attributes");
               if( (startp = strstr((char *)startp,substr) ) != NULL )
        {
                    leftp   = strstr((char *)startp,"{"); // �ҵ�������
                    rightp  = strstr((char *)startp,"}"); // �ҵ��һ�����
                    if( (leftp != NULL) && (rightp != NULL))
                    {
                            strncpy((char *)OTA_Info.recv_buf,leftp,rightp-leftp+1); // �����ַ���
                            printf("jsonbuf2=%s\n",OTA_Info.recv_buf);
                            MQTT_Parse_OTAData((uint8_t*)OTA_Info.recv_buf);// ����json�ַ����������豸
                            return SET;
                    }
                    //startp = startp + strlen(substr); // �ƶ�λ��

        }

    /***********************************************************/
    // �����������·���OTA�̼�����
    memset(substr,0,sizeof(substr));
    startp = (char *)recvbuf ;
    sprintf(substr,"v2/fw/response/%d/chunk/%d,%d,",
    OTA_Info.request_id,OTA_Info.chunk_id,OTA_Info.request_bytes) ;
    printf("ota:substr=%s\n",substr);
    if( (startp = strstr((char *)startp,substr) ) != NULL )
    {
        memcpy(OTA_Info.recv_buf,startp+strlen(substr),OTA_Info.request_bytes); // ���ƹ̼�����������
        printf("OTA chunk recv: id=%lu bytes=%lu\n",OTA_Info.chunk_id,OTA_Info.request_bytes);
        OTA_Info.recv_flag = 1; //  OTA ���յ��̼�������
        return SET;
        //MQTT_Parse_OTAData((uint8_t*)OTA_Info.recv_buf);// ����json�ַ����������豸
        //startp = startp + strlen(substr); // �ƶ�λ��
    }
        return RESET;
}
/*
 * ��������������豸�·�������json�������ڿ�������gpio
 * @param json ����json����
 * @return uint8_t 0 �ɹ� 1 ʧ��
 */
uint8_t MQTT_Parse_DeviceData(uint8_t *json,uint32_t dataNum)
{

    cJSON *cjson_device = NULL;
    cJSON *cjson_method = NULL;
    cJSON *cjson_params = NULL;
    /* ��������JSON���� */
    cjson_device = cJSON_Parse((char *)json);
    if (cjson_device == NULL)
    {
        printf("parse fail.\n");
        return RESET;
    }
    else
    {
       // printf("json->%s\n",cJSON_Print(cjson_device));
    }
    /* ���θ���������ȡJSON���ݣ���ֵ�ԣ� */
    cjson_method  = cJSON_GetObjectItem(cjson_device, "method");
    cjson_params  = cJSON_GetObjectItem(cjson_device, "params");
    //printf("method:%s\n",cjson_method->valuestring);
    //printf("params:%d\n",cjson_params->valueint);

    // ��ȡLED1��״̬

    if(strcmp(cjson_method->valuestring,"led1Status") == 0 ) {
        // ���������º�,���� ��Ϊ�͵�ƽ
        if (HAL_GPIO_ReadPin(LED1_GPIO_Port, LED1_Pin) == GPIO_PIN_RESET)
        {
            MQTT_SendData((uint8_t *)PUB2,dataNum,"true");
        }else {
            MQTT_SendData((uint8_t *)PUB2,dataNum,"false");
        }
    } // ��ȡbeep��״̬
    else if(strcmp(cjson_method->valuestring,"beepStatus") == 0 ) {
                // ���������º�,���� ��Ϊ�͵�ƽ
                if (HAL_GPIO_ReadPin(beep_GPIO_Port, beep_Pin) == GPIO_PIN_RESET)
                {
                        MQTT_SendData((uint8_t *)PUB2,dataNum,"true");
                }else {
                        MQTT_SendData((uint8_t *)PUB2,dataNum,"false");
                }
    }
        // ��ȡ�̵�����״̬
    else if(strcmp(cjson_method->valuestring,"relayStatus") == 0 ) {
                // ESC ���������º�,���� ��Ϊ�͵�ƽ
                if (HAL_GPIO_ReadPin(jdq_GPIO_Port, jdq_Pin) == GPIO_PIN_SET)
                {
                        MQTT_SendData((uint8_t *)PUB2,dataNum,"true");
                }else {
                        MQTT_SendData((uint8_t *)PUB2,dataNum,"false");
                }
    }


        // ����LED1��״̬
        if(strcmp(cjson_method->valuestring,"led1State") == 0 )
        {
                if(cjson_params->valueint == 1 )
                {
                        //LED_Control(LED1, ON);
                        REG_HOLD_BUF[0]  |= LED1_CMD  ;   // ��0λ д 1
                        MQTT_SendData((uint8_t *)PUB2,dataNum,"true");
                }else if(cjson_params->valueint == 0 )
                {
                        //LED_Control(LED1, OFF);
                        REG_HOLD_BUF[0]  = REG_HOLD_BUF[0] & (~LED1_CMD)  ; // ��0λ д 0
                        MQTT_SendData((uint8_t *)PUB2,dataNum,"false");
                }
        }
        // ����BEEP��״̬
        else if(strcmp(cjson_method->valuestring,"beepState") == 0 )
        {
                if(cjson_params->valueint == 1 )
                {
                        //LED_Control(LED1, ON);
                        REG_HOLD_BUF[0]  |= BEEP_CMD  ;   // ��0λ д 1
                        MQTT_SendData((uint8_t *)PUB2,dataNum,"true");
                }else if(cjson_params->valueint == 0 )
                {
                        //LED_Control(LED1, OFF);
                        REG_HOLD_BUF[0]  = REG_HOLD_BUF[0] & (~BEEP_CMD)  ; // ��0λ д 0
                        MQTT_SendData((uint8_t *)PUB2,dataNum,"false");
                }
        }

        // ����relay��״̬
        if(strcmp(cjson_method->valuestring,"relayState") == 0 )
        {
                if(cjson_params->valueint == 1 )
                {
                        //LED_Control(LED1, ON);
                        REG_HOLD_BUF[0]  |= RELAY_CMD  ;   // ��0λ д 1
                        MQTT_SendData((uint8_t *)PUB2,dataNum,"true");
                }else if(cjson_params->valueint == 0 )
                {
                        //LED_Control(LED1, OFF);
                        REG_HOLD_BUF[0]  = REG_HOLD_BUF[0] & (~RELAY_CMD)  ; // ��0λ д 0
                        MQTT_SendData((uint8_t *)PUB2,dataNum,"false");
                }
        }

        cJSON_Delete(cjson_device); // һ��Ҫ�ͷ��ڴ�
        return SET;
}
/*
 * �������ݵ�������
 * @param topic ����
 * @param dataNum ��������
 * @param status ״̬
 * @return uint8_t 0 �ɹ� 1 ʧ��
 */
uint8_t MQTT_SendData(uint8_t *topic,uint32_t dataNum,char *status)
{
    uint8_t sendbuf[128] = {0};
    uint8_t buf[256] = {0};

    if(strncmp((char *)topic,PUB1,strlen(PUB1)-1) == 0 )
    {

        if(dataNum == 1) // �ϴ�����������
        {
            // MQPUB,1,v1/devices/me/telemetry
            sprintf((char *)sendbuf,"{TP:%d,RH:%d,VO:%d,CU:%d,PW:%d,VR:%d,CPU:%d}",
            REG_HOLD_BUF[1],REG_HOLD_BUF[2], REG_HOLD_BUF[3],REG_HOLD_BUF[4],
            REG_HOLD_BUF[5],REG_HOLD_BUF[6],REG_HOLD_BUF[7]);
            // ��������1   �ϴ����������ݵ�  "v1/devices/me/telemetry"
            sprintf((char *)buf,"MQPUB,1,%s,%s",PUB1,sendbuf);
        }
        else if(dataNum == 2)  // ota ״̬����
        {
            sprintf((char *)sendbuf,"{\"fw_state\":\"UPDATED\"}");
            // ��������1   �ϴ����������ݵ�  "v1/devices/me/telemetry"
            sprintf((char *)buf,"MQPUB,1,%s,%s",PUB1,sendbuf);
        }
        else if(dataNum == 3)  // ota ״̬����
        {
            sprintf((char *)sendbuf,"{\"fw_state\":\"FAILED\"}");
            // ��������1   �ϴ����������ݵ�  "v1/devices/me/telemetry"
            sprintf((char *)buf,"MQPUB,1,%s,%s",PUB1,sendbuf);
        }

    }
    else if(strncmp((char *)topic,PUB2,strlen(PUB2)-1) == 0 )
    {
        // ��������2   �ظ��������·������� PUB2  "v1/devices/me/rpc/response/"
        sprintf((char *)buf,"MQPUB,1,%s%d,%s",PUB2,dataNum,status);
    }
//MQTT_SendData((uint8_t *)PUB3,OTA_Info.request_id,strNumber);
    else if(strncmp((char *)topic,PUB3,strlen(PUB3)-1) == 0 )
    {
        // ��������3   �������ع̼����� "v2/fw/request/+/chunk/#"
        sprintf((char *)buf,"MQPUB,1,v2/fw/request/%lu/chunk/%lu,%s",
        OTA_Info.request_id,OTA_Info.chunk_id,status);
        // ��������3   �������ع̼����� "v2/fw/request/+/chunk/#"  AT+MQTTPUB=0,"v2/fw/request/143488/chunk/0",1,0,0,3,"256"\r\n
        //sprintf((char *)buf,"MQPUB,1,v2/fw/request/%lu/chunk/%lu/,%s\r\n",
        //sprintf((char *)buf,"AT+MQTTPUB=0,\"v2/fw/request/%lu/chunk/%lu\",1,0,0,3,"256"\r\n",
        //OTA_Info.request_id,OTA_Info.chunk_id,status);//AT+MQTTPUB=0,"v2/fw/request/143488/chunk/0",1,0,0,3,"256"\r\n
    }
    printf("MQTT Send Data:%s",buf);
    HAL_UART_Transmit(&huart2,(uint8_t*)buf,strlen((char *)buf),1000);
    return SET;
}
/*
 * ����OTA �� json����
 * @param json OTA����
 * @return uint8_t 0 �ɹ� 1 ʧ��
 */

uint8_t MQTT_Parse_OTAData(uint8_t *json) {
        cJSON *cjson_device = NULL;
        cJSON *  cjson_title =NULL;
        cJSON *  cjson_version=NULL;
        cJSON *    cjsonsize =NULL;
        cJSON *cjson_checksum=NULL;
        cjson_device = cJSON_Parse((char *)json);
        if (cjson_device==NULL) {
            printf("cJSON_Parse fail\r\n");
            return RESET;
        }
    else {

    }
    cjson_title=cJSON_GetObjectItem(cjson_device,"fw_title");
    if (cjson_title==NULL){return RESET;}
    cjson_version=cJSON_GetObjectItem(cjson_device,"fw_version");
    cjsonsize=cJSON_GetObjectItem(cjson_device,"fw_size");
    cjson_checksum=cJSON_GetObjectItem(cjson_device,"fw_checksum");
    /* char fw_title[32];
    char fw_version[32];
    uint32_t fw_size;
    char fw_checksum[32];
    */
    if ((cjson_version == NULL) || (cjsonsize == NULL) || (cjson_checksum == NULL)) {
        cJSON_Delete(cjson_device);
        return RESET;
    }
    strncpy(OTA_Info.fw_title,cjson_title->valuestring,sizeof(OTA_Info.fw_title)-1);
    strncpy(OTA_Info.fw_version,cjson_version->valuestring,sizeof(OTA_Info.fw_version)-1);
    strncpy(OTA_Info.fw_checksum,cjson_checksum->valuestring,sizeof(OTA_Info.fw_checksum)-1);
    OTA_Info.fw_size=cjsonsize->valueint;
    MQTT_OTA_Flag=1;
    printf("OTA_Info.fw_title=%s\r\n",OTA_Info.fw_title);
    printf("OTA_Info.fw_version=%s\r\n",OTA_Info.fw_version);
    printf("OTA_Info.fw_size=%d\r\n",OTA_Info.fw_size);
    printf("OTA_Info.received_bytes=%d\r\n",OTA_Info.received_bytes);
    printf("OTA_Info.fw_checksum=%s\r\n",OTA_Info.fw_checksum);
    printf("MQTT_OTA_Flag=1\r\n");
    cJSON_Delete(cjson_device);
    return SET;
}

// ÿ�յ����ݰ�ʱ���ã�len=256�����һ����
void Update_Progress(uint32_t len) {
    OTA_Info.received_bytes += len;
    uint8_t percent = (OTA_Info.received_bytes * 100) / OTA_Info.fw_size;
    char progress[20] = { 0 };
    if(percent > 100 ) percent = 100;
    if(percent == 100)
    {
        sprintf(progress,"OTA%s        OK",OTA_Info.fw_version);
    }
    else
    {
        sprintf(progress,"OTA%s      %3d%%",OTA_Info.fw_version,percent);
    }
}

uint8_t MQTT_OTA_GetFW(void)
{
    uint32_t APP_DownloadNew_Address=0;
    uint32_t APP_DownloadOld_Address=0;
        uint32_t size = OTA_Info.fw_size/256 ;
        if(OTA_Info.fw_size%256 != 0 )
        {
                size++;
        }
        HAL_IWDG_Refresh(&hiwdg);  // 擦除前喂狗, Flash擦除期间CPU停顿无法喂狗
        if (MyFlash_ReadWord(Application_1_Addr + Application_Size - 4) == 0xAAAAAAAA) {
            MyFlash_Erase(APP2_Flash_Bank,OTA_Info.fw_size);
            APP_DownloadNew_Address=0x08040000U;//新固件下载地址
            APP_DownloadOld_Address=0x08008000U;//旧固件地址
            printf("Erase APP2 ...|size=%d",size);
        }else if (MyFlash_ReadWord(Application_2_Addr + Application_Size - 4) == 0xAAAAAAAA){//app2更新标志位为0xAAAAAAAA表明在运行中，需要擦除APP1
            MyFlash_Erase(APP1_Flash_Bank,OTA_Info.fw_size);
            APP_DownloadNew_Address=0x08008000U;//新固件下载地址
            APP_DownloadOld_Address=0x08040000U;//旧固件地址
            printf("Erase APP1 ...|size=%d",size);
        }
        //MyFlash_Erase(APP2_Flash_Bank,OTA_Info.fw_size);
        OTA_Info.received_bytes = 0 ;
        OTA_Info.recv_flag = 0 ;
        OTA_Info.request_id = HAL_GetTick() + (rand() & 0xFFFF); ;
        OTA_Info.chunk_id  = 0 ;
        OTA_Info.request_bytes = 256;
        CRC32_Init(&crc_ctx);
        for(uint32_t i =0;i < size ; i++,OTA_Info.chunk_id++)
        {
            HAL_IWDG_Refresh(&hiwdg) ; // 刷新看门狗
                char strNumber[10]={0};
                if( (i == size -1) && (OTA_Info.fw_size%256 != 0) )
                {
                        OTA_Info.request_bytes = OTA_Info.fw_size%256 ;
                }
                sprintf(strNumber,"%d",256);
                OTA_Info.recv_flag = 0 ;
                MQTT_SendData((uint8_t *)PUB3,OTA_Info.request_id,strNumber);
                uint32_t Timeout = 10*1000;
                while (OTA_Info.recv_flag == 0)
                {
                    if ((Timeout--) == 0){
                        printf("OTA Timeout\r\n");
                        printf("升级失败,擦除");
                        if (MyFlash_ReadWord(Application_1_Addr + Application_Size - 4) == 0xAAAAAAAA) {
                            MyFlash_Erase(APP2_Flash_Bank,OTA_Info.fw_size);
                            printf(" APP2扇区 \r\n");
                        }else if (MyFlash_ReadWord(Application_2_Addr + Application_Size - 4) == 0xAAAAAAAA){
                            MyFlash_Erase(APP1_Flash_Bank,OTA_Info.fw_size);
                            printf(" APP1扇区 \r\n");
                        }
                        return RESET;
                    }
                    HAL_Delay(1);
                    HAL_IWDG_Refresh(&hiwdg);  // 等待MQTT响应期间喂狗
                }
                printf("Timeout=%d",10*1000-Timeout);
                OTA_Info.recv_flag = 0 ;
                Update_Progress(OTA_Info.request_bytes);
                if( (i == size -1) && (OTA_Info.fw_size%256 != 0) )
                {
                    MyFlash_Save(APP_DownloadNew_Address + i * 256,(char *)OTA_Info.recv_buf,OTA_Info.request_bytes);
                    CRC32_Update(&crc_ctx,OTA_Info.recv_buf,OTA_Info.request_bytes);
                }
                else
                {
                    CRC32_Update(&crc_ctx, OTA_Info.recv_buf, 256);  // 增量更新CRC
                    MyFlash_Save(APP_DownloadNew_Address + i * 256,(char *)OTA_Info.recv_buf,256);
                }
        }
    // 进行crc32 校验
    uint32_t crc32 =  CRC32_Final(&crc_ctx,1);
    printf("client_crc32=%x\n",crc32);
    char  crcstr[10] = {0};
    sprintf(crcstr,"%x",crc32);
    printf("server_crc32=%s\n",OTA_Info.fw_checksum);
    if(strncmp(OTA_Info.fw_checksum,crcstr,strlen(OTA_Info.fw_checksum)) == 0 )
        {
                MQTT_SendData((uint8_t *)PUB1,2,NULL);
                uint32_t update1_flag = 0x00000000;
                MyFlash_Save(APP_DownloadOld_Address + Application_Size - 4, (char *)&update1_flag,4);//写入更新标志位到旧固件
                uint32_t update2_flag = 0xAAAAAAAA;
                MyFlash_Save(APP_DownloadNew_Address + Application_Size - 4, (char *)&update2_flag,4);//写入更新标志位到新固件
                HAL_NVIC_SystemReset();
        }
        else
        {
                MQTT_SendData((uint8_t *)PUB1,3,NULL);
                printf("CRC Fail,擦除下载区\r\n");
                if (MyFlash_ReadWord(Application_1_Addr + Application_Size - 4) == 0xAAAAAAAA) {
                    MyFlash_Erase(APP2_Flash_Bank,OTA_Info.fw_size);
                }else if (MyFlash_ReadWord(Application_2_Addr + Application_Size - 4) == 0xAAAAAAAA){
                    MyFlash_Erase(APP1_Flash_Bank,OTA_Info.fw_size);
                }
                MQTT_OTA_Flag = 1;
                return RESET ;
        }
        return SET;
}
