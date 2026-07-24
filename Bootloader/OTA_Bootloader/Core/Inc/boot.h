//
// Created by Lenovo on 2026/7/12.
//

#ifndef __BOOT_H_
#define __BOOT_H_
#define PageSize FLASH_PAGE_SIZE // 1K
/*=====用户配置(根据自己的分区进行配置)=====*/
// 0x400    1K
// 0x800    2K
// 0x1000   4K
// 0x2000   8
// 0x7000   4K*7 = 28K
#define BootLoader_Size         0x8000U         // BootLoader的大小 32K  扇区0+扇区1 0x0800 0000--0x0800 7FFF
#define App1lication_Size        0x38000U         // APP 应用程序的大小224K
#define App2lication_Size       0x38000U        // APP2的大小 224K
#define Application_1_Addr      0x08008000U     // 应用程序1的首地址 大小 224K（16k+16k+64k+128k）扇区2 3 4 5
#define Application_2_Addr      0x08040000U     // 应用程序2的首地址 大小  256k 0x0804 0000 + 4 0000扇区 6 7
/*==========================================*/
/* 启动的步骤 */

#define Startup_1 0x1   //  从APP1启动
#define Startup_2 0x2   //  从APP2启动
void Start_BootLoader(void);
#endif
