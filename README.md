#OTA工程
**本工程采用双区OTA，APP1为出厂程序,APP2为OTA中的B区程序APP3为OTA的A区程序**
本工程已将编译成功在build文件夹下分别有Dabug(O0优化)和Release(Os优化)
本工程利用gcc+openocd+Cmake
实现了远程OTA+远程监测上传，以及双区OTA,看门狗可防止固件变砖
