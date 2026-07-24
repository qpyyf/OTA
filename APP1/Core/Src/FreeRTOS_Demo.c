#include "FreeRTOS_Demo.h"
#include "FREERTOS.h"
#include "TASK.h"
#include "GPIO.h"
#include "stdio.h"
#include "stm32f4xx_it.h"
#include "mb.h"
#include "modbus.h"
#include "mqtt.h"
#include "wifi4g.h"
#include "canfestival.h"
#include "flash.h"
#include "testslave.h"
#include "tim.h"
#include "main.h"
#include "../../Core/Inc/can.h"

#define TASK_START_STACK 128
#define TASK_START_PRIORITY 1
TaskHandle_t Task_Start_Handle;
void Task_Start(void *  pvParameters);

#define TASK1_STACK 128
#define TASK1_PRIORITY 2
TaskHandle_t Task1_Handle;
void Task1(void *  pvParameters);

#define TASK2_STACK 128
#define TASK2_PRIORITY 4
TaskHandle_t Task2_Handle;
void Task2(void *  pvParameters);

#define TASK3_STACK 256
#define TASK3_PRIORITY 4
TaskHandle_t Task3_Handle;
void Task3(void *  pvParameters);

#define TASK4_STACK 768
#define TASK4_PRIORITY 4
TaskHandle_t Task4_Handle;
void Task4(void *  pvParameters);



void FreeRTOS_Start(void)
{
	taskENTER_CRITICAL();
	xTaskCreate( (TaskFunction_t) Task_Start,
	                            "Task_Start",
	                            TASK_START_STACK,
	                            (void *)NULL,
	                            TASK_START_PRIORITY,
	                            (TaskHandle_t *) &Task_Start_Handle );
	Modbus_Init();

	vTaskStartScheduler();
	taskEXIT_CRITICAL();
}

void Task_Start(void *  pvParameters)
{
	xTaskCreate( (TaskFunction_t) Task1,
	                            "Task1",
	                            TASK1_STACK,
	                            (void *)NULL,
	                            TASK1_PRIORITY,
	                            (TaskHandle_t *) &Task1_Handle );
	xTaskCreate( (TaskFunction_t) Task2,
	                            "Task2",
	                            TASK2_STACK,
	                            (void *)NULL,
	                            TASK2_PRIORITY,
	                            (TaskHandle_t *) &Task2_Handle );
	xTaskCreate( (TaskFunction_t) Task3,
	                            "Task3",
	                            TASK3_STACK,
	                            (void *)NULL,
	                            TASK3_PRIORITY,
	                            (TaskHandle_t *) &Task3_Handle );
	xTaskCreate( (TaskFunction_t) Task4,
	                            "Task4",
	                            TASK4_STACK,
	                            (void *)NULL,
	                            TASK4_PRIORITY,
	                            (TaskHandle_t *) &Task4_Handle );
	vTaskDelete(NULL);
}

void Task1(void *  pvParameters)
{
	CanFestival_Can_Init(); //初始化can过滤器
	setNodeId(&TestSlave_Data, SlavemodbusAddress);
	setState(&TestSlave_Data, Initialisation);
	setState(&TestSlave_Data, Operational);
	while(1) {
		// GPIO_Control(LED1,OFF);
		// vTaskDelay(500);
		// GPIO_Control(LED1,ON);
		vTaskDelay(500);
	}
}

void Task2(void *  pvParameters)
{
	while(1) {

		// GPIO_Control(LED2,OFF);
		// vTaskDelay(300);
		// GPIO_Control(LED2,ON);
		vTaskDelay(500);
	}
}


void Task3(void *  pvParameters)
{

	while(1) {
		eMBPoll();
		Modbus_parse();
		vTaskDelay(10);//10ms刷新一次
	}
}

void Task4(void *  pvParameters)
{

	Queue_Init(&QUART2);
	if (MQTT_Connect_MQTTServer() == SET) {
		MQTTDownLoad_Flag=1;//开始接收下行命令，置标志位处理（开始解析的标志位）
		HAL_TIM_Base_Start_IT(&htim3);//启动定时器3中断500ms中断用于刷新modbus *Modbus_parse* 5s用于mqtt上传数据
	}
		while (1) {
		//ESP8266_Test();
		if (MQTTUpLoad_flag==1) {
			MQTTUpLoad_flag=0;
			MQTT_SendseniorData();
			//MQTT_SendData(PUB1,1,NULL);// 发送数据 pub1上传通道，1表示上传数据
		}
		/*
		 * 开始进行OTA更新
		 *接受固件并擦除 flash校验 crc32是否一致
		 */
		if (MQTT_OTA_Flag==1) {
			MQTT_OTA_Flag=0;
			//暂停freertos任务，等待OTA更新完成
			 vTaskSuspend(Task1_Handle);
			 vTaskSuspend(Task2_Handle);
			 vTaskSuspend(Task3_Handle);
			//taskENTER_CRITICAL();
			//OTA更新完成后，继续执行
			MQTT_OTA_GetFW();
			//taskEXIT_CRITICAL();
			vTaskResume(Task1_Handle);
			vTaskResume(Task2_Handle);
			vTaskResume(Task3_Handle);
		}
		vTaskDelay(500);//50ms刷新一次
	}
}
