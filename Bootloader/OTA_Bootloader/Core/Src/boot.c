//
// Created by Lenovo on 2026/7/12.
//

#include "boot.h"
#include "Flash.h"
#include "main.h"
#include <stdio.h>
#include "stm32f4xx.h"
/**
 * @bieaf ����ҳ
 *
 * @param pageaddr  ��ʼ��ַ
 * @param num       ������ҳ��
 * @return 1
 */
// static int Erase_page(uint32_t pageaddr, uint32_t num)
// {
//     HAL_FLASH_Unlock();
//
//     /* ����FLASH*/
//     FLASH_EraseInitTypeDef FlashSet;
//     FlashSet.TypeErase = FLASH_TYPEERASE_PAGES;
//     FlashSet.PageAddress = pageaddr;
//     FlashSet.NbSectors = num;
//
//     /*����PageError�����ò�������*/
//     uint32_t PageError = 0;
//     HAL_FLASHEx_Erase(&FlashSet, &PageError);
//
//     HAL_FLASH_Lock();
//     return 1;
// }
/**
 * @bieaf д���ɸ�����
 *
 * @param addr       д��ĵ�ַ
 * @param buff       д�����ݵ���ʼ��ַ
 * @param word_size  ����
 * @return
 */
static void WriteFlash(uint32_t addr, uint32_t * buff, int word_size)
{
    /* 1/4����FLASH*/
    HAL_FLASH_Unlock();

    for(int i = 0; i < word_size; i++)
    {
        /* 3/4��FLASH��д*/
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + 4 * i, buff[i]);
    }
    /* 4/4��סFLASH*/
    HAL_FLASH_Lock();
}
/**
 * @bieaf �����ɸ�����
 *
 * @param addr       �����ݵĵ�ַ
 * @param buff       �������ݵ�����ָ��
 * @param word_size  ����
 * @return
 */
static void ReadFlash(uint32_t addr, uint32_t * buff, uint16_t word_size)
{
    for(int i =0; i < word_size; i++)
    {
        buff[i] = *(__IO uint32_t*)(addr + 4 * i);
    }
    return;
}
/* ��ȡ����ģʽ */
unsigned int Read_Start_Mode(void)
{
    uint32_t app2mode = 0;
    uint32_t app1mode = 0;
    uint32_t app2runmode = 0;
    uint32_t app1runmode = 0;
    uint32_t iwdg_reset_flag = __HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST);  // 先保存
    __HAL_RCC_CLEAR_RESET_FLAGS();  // 再清除, 防止标志永久置位
    ReadFlash((Application_2_Addr + App2lication_Size - 4), &app2mode, 1);
    ReadFlash((Application_1_Addr + App1lication_Size - 4), &app1mode, 1);
    ReadFlash((Application_2_Addr + App2lication_Size - 8), &app2runmode, 1);
    ReadFlash((Application_1_Addr + App1lication_Size - 8), &app1runmode, 1);
    /*****************************看门狗复位恢复逻辑(最高优先级)******************************/
    if (iwdg_reset_flag == SET){
        if (app1mode==0x00000000&&app2mode==0xAAAAAAAA) {
            //看门狗复位说明app2已崩溃,恢复启动app1
            printf("APP2 damaged, startup APP1.\r\n");
            uint32_t APP1_DogREST=0x55555555;//app1看门狗恢复标志
            MyFlash_Save(Application_1_Addr+App1lication_Size-8,(char *)&APP1_DogREST,4);
            return Startup_1;
        }
        if (app1mode==0xAAAAAAAA&&app2mode==0x00000000) {
            //看门狗复位说明app1已崩溃,恢复启动app2
            uint32_t APP2_DogREST=0x55555555;//app2看门狗恢复标志
            MyFlash_Save(Application_2_Addr+App2lication_Size-8,(char *)&APP2_DogREST,4);
            printf("APP1 damaged, startup APP2.\r\n");
            return Startup_2;
        }
    }
    /*****************************正常复位时的处理逻辑******************************/
    //app正常运行时会把自己的runmode设置为0xAAAAAAAA
    if(app1mode==0xAAAAAAAA){//app1正常运行中复位
        return Startup_1;
    }
    if(app2mode==0xAAAAAAAA) {
        //app2正常运行中复位
        return Startup_2;
    }
    /************************OTA完成时的处理逻辑*********************************/
    if(app2mode==0xFFFFFFFF&&app1mode==0xFFFFFFFF){//初始状态
        return Startup_1;
    }
    if (app1mode==0x00000000&&app2mode==0xAAAAAAAA) {//OTA完成后app2启动
        return Startup_2;
    }
    if (app1mode==0xAAAAAAAA&&app2mode==0x00000000) {//OTA完成后app1启动
        return Startup_1;
    }
    return Startup_1;
}
/**
 * @bieaf ���г���ĸ���
 * @detail 1.����Ŀ�ĵ�ַ
 *         2.Դ��ַ�Ĵ��뿽����Ŀ�ĵ�ַ
 *         3.����Դ��ַ
 *
 * @param  ���˵�Դ��ַ
 * @param  ���˵�Ŀ�ĵ�ַ
 * @return ���˵ĳ����С
 */
void MoveCode(uint32_t src_addr, uint32_t des_addr, uint32_t byte_size)
{
    /*1.����Ŀ�ĵ�ַ*/
    printf("> Start erase des flash......\r\n");
    //Erase_page(des_addr, (byte_size/PageSize));
    MyFLASH_Erase(2, 4, 0);//����APP1
    printf("> Erase des flash down......\r\n");

    /*2.��ʼ����*/
    uint32_t temp[256];

    printf("> Start copy......\r\n");
    for(int i = 0; i < byte_size/1024; i++)
    {
        ReadFlash((src_addr + i*1024), temp, 256);
        WriteFlash((des_addr + i*1024), temp, 256);
    }
    printf("> Copy down......\r\n");

    /*3.����Դ��ַ*/
    printf("> Start erase src flash......\r\n");
    //Erase_page(src_addr, (byte_size/PageSize));
    MyFLASH_Erase(6,2,0 );
    HAL_Delay(10);
    printf("> Erase src flash down......\r\n");
}

/* ������ת���� */
typedef void (*Jump_Fun)(void);
void IAP_ExecuteApp (uint32_t App_Addr)
{
    Jump_Fun JumpToApp;
//stm32f407vgt6��ջ����ַΪ0x20000000ջ��СΪ128KB��0x2002 0000������Ҫ��0x2FFD0000���������0010��2��1100(c)
if ( ( ( * ( __IO uint32_t * ) App_Addr ) & 0x2FFD0000 ) == 0x20000000 )    //���ջ����ַ�Ƿ�Ϸ�.
    {
        printf("> Stack pointer is valid.\r\n");

        HAL_DeInit();//ȡ����ʼ��HAL
        __disable_irq();
        SysTick->CTRL = 0;//�ر�SysTick

        __set_MSP((*(__IO uint32_t *)App_Addr));
        JumpToApp = (Jump_Fun) * ( __IO uint32_t *)(App_Addr + 4);  //�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)
        //MSR_MSP( * ( __IO uint32_t * ) App_Addr );  //��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)

        JumpToApp();        //��ת��APP.��ת��flash��APP��4��λ��
    }
    else
    {
        printf("> Stack pointer is not.\r\n");
    }
}
/**
 * @bieaf ����BootLoader������
 *
 * @param none
 * @return none
 */
__IO uint32_t App_Addr=0;
void Start_BootLoader(void)
{
    // /*==========��ӡ��Ϣ==========*/
    //  printf("\r\n");
    //  printf("***********************************\r\n");
    //  printf("*        BootLoader               *\r\n");
    //  printf("***********************************\r\n");


    // //printf("> Choose a startup method......\r\n");
    //OTA���º�APP2��д��0xAAAAAAAA
    //��ȡ�ж��Ƿ���Ҫ����
    //˫��OTAҪ��ת����ͬ����APP

    switch(Read_Start_Mode())                                       //  ��ȡ�Ƿ�����Ӧ�ó���
    {
        case Startup_1:                                        //  ��������
        {
            printf("> APP1...\n");
            //MyFLASH_Erase(6,2,0);
            App_Addr=Application_1_Addr;
            break;
        }
        case Startup_2:                                        // ��APP2����
        {
            printf("> APP2...\n");
            App_Addr=Application_2_Addr;
            break;
        }
        default:                                                    //  ����ʧ��
        {
            printf("> Error!!!......\r\n");
            App_Addr=Application_1_Addr;
            break;
        }
    }
    printf("> Start APP1...\r\n");
    IAP_ExecuteApp(App_Addr);


    // IAP_ExecuteApp(Application_1_Addr);
}
