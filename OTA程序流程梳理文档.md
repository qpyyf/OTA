# STM32F407 OTA 双区升级程序流程梳理文档

---

## 一、系统总览思维导图

```
                        ┌───────────────────────────────────────────────────────────────┐
                        │                 STM32F407 双区 OTA 系统                          │
                        └───────────────────────────────────────────────────────────────┘
                                                    │
        ┌───────────────────────────────────────────┼───────────────────────────────────────────┐
        │                                           │                                           │
   ┌────▼────┐                                ┌────▼────┐                                ┌────▼────┐
   │ 硬件层   │                                │ 通信层   │                                │ 存储层   │
   └────┬────┘                                └────┬────┘                                └────┬────┘
        │                                           │                                           │
   ┌────▼────┐                              ┌───────▼───────┐                            ┌──────▼──────┐
   │STM32F407│                              │ 4G/WiFi 模块   │                            │  Flash 布局  │
   │ VGT6    │                              │ (ML307/ESP8266)│                            │  1MB Total  │
   │ 168MHz  │                              │ UART2 通信     │                            └──────┬──────┘
   └─────────┘                              └───────┬───────┘                                   │
                                                    │                                    ┌──────▼──────┐
                                            ┌───────▼───────┐                            │ Bootloader  │
                                            │   MQTT 协议    │                            │ 0x08000000  │
                                            │ broker.emqx.io │                            │ 扇区 0-1    │
                                            └───────┬───────┘                            │ 32KB        │
                                                    │                                    └──────┬──────┘
                                          ┌─────────┼─────────┐                                │
                                          │         │         │                         ┌──────▼──────┐
                                    下行主题1  下行主题2  下行主题3                       │ APP1 (A区)  │
                                    (RPC控制)  (OTA信息)  (固件分块)                     │ 0x08008000  │
                                          │         │         │                         │ 扇区 2-5    │
                                    上行主题1  上行主题2  上行主题3                       │ 224KB       │
                                    (遥测数据) (RPC回复)  (请求分块)                     └──────┬──────┘
                                                                                              │
                                                                                       ┌──────▼──────┐
                                                                                       │ APP2 (B区)  │
                                                                                       │ 0x08040000  │
                                                                                       │ 扇区 6-7    │
                                                                                       │ 224KB       │
                                                                                       └──────┬──────┘
                                                                                              │
                                                                                       ┌──────▼──────┐
                                                                                       │   DATA 区   │
                                                                                       │ 0x08060000  │
                                                                                       │ 用户数据    │
                                                                                       └─────────────┘


              ┌───────────────────────────────────────────────────────────────────────────────┐
              │                              软件架构 (FreeRTOS)                                │
              └───────────────────────────────────────────────────────────────────────────────┘

                                                ┌─────────────┐
                                                │   main()    │
                                                │ 系统初始化   │
                                                └──────┬──────┘
                                                       │
                                          ┌────────────▼────────────┐
                                          │    FreeRTOS_Start()     │
                                          │  创建 4 个任务            │
                                          └────────────┬────────────┘
                                                       │
              ┌────────────────────────────────────────┼────────────────────────────────────────┐
              │                    │                   │                    │                   │
         ┌────▼────┐         ┌────▼────┐         ┌────▼────┐         ┌────▼────┐
         │ Task1   │         │ Task2   │         │ Task3   │         │ Task4   │   ← MQTT/OTA主任务
         │ 优先级  │         │ 优先级  │         │ 优先级  │         │ 优先级  │
         │ APP1:2  │         │ 均为4  │         │ 均为4  │         │ 均为4  │
         │ APP2/3:4│         │        │         │        │         │        │
         └────┬────┘         └────┬────┘         └────┬────┘         └────┬────┘
              │                    │                   │                    │
         CANOpen总线     APP1:空闲循环         Modbus轮询         ┌───────┴───────┐
                         APP2/3:喂看门狗
                                                                     │               │
                                                               MQTT连接管理      OTA下载控制
                                                               (连接到服务器)    (MQTT_OTA_GetFW)


              ┌───────────────────────────────────────────────────────────────────────────────┐
              │                     数据接收链路 (UART2 → MQTT → 业务逻辑)                        │
              └───────────────────────────────────────────────────────────────────────────────┘

     4G/WiFi模块                    UART2 DMA                      队列 QUART2
     ┌─────────┐    串口数据     ┌─────────────┐   半满/全满/空闲   ┌─────────────┐
     │ ML307 / │ ──────────────→ │ DMA 自动搬运  │ ──────────────→ │  FIFO 队列   │
     │ ESP8266 │                 │ 到 RX2_Buf   │  中断触发入队     │  (1024字节)  │
     └─────────┘                 └─────────────┘                   └──────┬──────┘
                                                                         │
                                                              TIM5 中断 10ms
                                                              (WIFI4G_Parse_Queue)
                                                                         │
                                                          ┌──────────────▼──────────────┐
                                                          │                            │
                                                     MQTT连接前                    MQTT连接后
                                                     (解析AT指令                   (MQTTDownLoad_Flag=1)
                                                      查找 "OK"/"ERROR")           CAT1_Parse_ATComander()
                                                                                         │
                                                                     ┌───────────────────┼───────────────────┐
                                                                     │                   │                   │
                                                               RPC设备控制           OTA固件信息          OTA固件分块
                                                               (v1/devices/me/     (v1/devices/me/    (v2/fw/response/)
                                                                rpc/request/)        attributes)          → OTA_Info.recv_buf
                                                                     │                   │                   │
                                                              MQTT_Parse_DeviceData  MQTT_Parse_OTAData    OTA_Info.recv_flag=1
                                                              控制 LED/BEEP/继电器   设置 MQTT_OTA_Flag=1


              ┌───────────────────────────────────────────────────────────────────────────────┐
              │                           OTA 升级完整流程                                      │
              └───────────────────────────────────────────────────────────────────────────────┘

    服务器下发固件信息 (attributes主题)
         │
         ▼
    MQTT_Parse_OTAData() 解析 JSON
    提取: fw_title, fw_version, fw_size, fw_checksum
    设置: MQTT_OTA_Flag = 1
         │
         ▼
    Task4 检测到 MQTT_OTA_Flag == 1
    暂停其他任务 (防止干扰):
      APP1: 暂停 Task1, Task2, Task3
      APP2/APP3: 暂停 Task1, Task3 (Task2 负责喂狗，不暂停)
    调用 MQTT_OTA_GetFW()
         │
         ▼
    ┌────────────────────────────────────────────────────────────┐
    │ 1. 判断当前运行区 (读两个APP的-4标志)                       │
    │    APP1 -4 == 0xAAAAAAAA → 下载到 APP2 (0x08040000)        │
    │    APP2 -4 == 0xAAAAAAAA → 下载到 APP1 (0x08008000)        │
    └──────────────────────────┬─────────────────────────────────┘
                               │
    ┌──────────────────────────▼─────────────────────────────────┐
    │ 2. 擦除目标区 Flash                                         │
    │    MyFlash_Erase() → 擦除整个目标APP区域                    │
    │    (擦除会自动将标志位重置为 0xFFFFFFFF)                      │
    └──────────────────────────┬─────────────────────────────────┘
                               │
    ┌──────────────────────────▼─────────────────────────────────┐
    │ 3. 逐块下载 (每块 256 字节)                                  │
    │    for chunk_id = 0 to total_chunks:                       │
    │      ① 通过 PUB3 主题请求服务器下发第 chunk_id 块           │
    │      ② 等待 OTA_Info.recv_flag == 1 (超时 10s)             │
    │      ③ 超时 → 擦除目标区 → 返回失败                        │
    │      ④ 写入 Flash (MyFlash_Save)                           │
    │      ⑤ 增量更新 CRC32                                      │
    └──────────────────────────┬─────────────────────────────────┘
                               │
                    ┌──────────▼──────────┐
                    │  4. CRC32 校验      │
                    │  本地 CRC vs 服务器  │
                    └──────┬──────┬──────┘
                           │      │
                    校验通过 │      │ 校验失败
                           │      │
              ┌────────────▼─┐  ┌─▼────────────┐
              │  CRC 一致     │  │  CRC 不一致    │
              │  ① 旧固件 -4  │  │  ① 擦除目标区  │
              │    写 0x00000 │  │  ② 发送 FAILED │
              │    000        │  │  ③ MQTT_OTA_  │
              │  ② 新固件 -4  │  │     Flag=1    │
              │    写 0xAAAAA │  │  ④ return     │
              │    AAAA       │  │               │
              │  ③ 发送       │  └───────────────┘
              │    UPDATED    │
              │  ④ NVIC_      │
              │    SystemRst()│
              └───────────────┘
                           │
                           ▼
                  系统复位 → Bootloader


              ┌───────────────────────────────────────────────────────────────────────────────┐
              │                          Bootloader 启动流程                                     │
              └───────────────────────────────────────────────────────────────────────────────┘

                        ┌──────────┐
                        │ 上电复位  │
                        └────┬─────┘
                             │
                  读取4个标志位:
                  APP1 -4 (运行), APP2 -4 (运行)
                  APP1 -8 (看门狗), APP2 -8 (看门狗)
                             │
                      ┌──────▼──────┐
                      │ IWDG 复位?   │
                      └──┬───────┬──┘
                     YES │       │ NO
                         │       │
              ┌──────────▼─┐     │
              │ APP2崩溃?   │     │
              │ (APP1=0x00  │     │
              │  APP2=0xAA) │     │
              │→ 写APP1 -8  │     │
              │  =0x5555    │     │
              │→ 启动APP1   │     │
              ├────────────┤     │
              │ APP1崩溃?   │     │
              │ (APP1=0xAA  │     │
              │  APP2=0x00) │     │
              │→ 写APP2 -8  │     │
              │  =0x5555    │     │
              │→ 启动APP2   │     │
              └────────────┘     │
                                 │
                    ┌────────────▼────────────┐
                    │  APP1 -4 == 0xAAAAAAAA?  │──→ 启动 APP1
                    ├─────────────────────────┤
                    │  APP2 -4 == 0xAAAAAAAA?  │──→ 启动 APP2
                    ├─────────────────────────┤
                    │  两个都是 0xFFFFFFFF?     │──→ 启动 APP1 (出厂)
                    ├─────────────────────────┤
                    │  APP1=0x00, APP2=0xAA   │──→ 启动 APP2 (OTA完成)
                    ├─────────────────────────┤
                    │  APP1=0xAA, APP2=0x00   │──→ 启动 APP1 (OTA完成)
                    ├─────────────────────────┤
                    │  默认                   │──→ 启动 APP1
                    └─────────────────────────┘
                             │
                             ▼
                      IAP_ExecuteApp()
                      ① 验证栈顶地址合法性
                      ② HAL_DeInit() / 关中断 / 停SysTick
                      ③ 设置 MSP (新APP栈顶)
                      ④ 跳转到新APP的Reset_Handler
```

---

## 二、系统硬件与存储架构

### 2.1 硬件平台

- **MCU**: STM32F407VGT6，ARM Cortex-M4，168MHz，1MB Flash，192KB SRAM
- **通信模块**: UART2 连接 4G 模块 (ML307) 或 WiFi 模块 (ESP8266)
- **外设总线**: UART1 (调试/主机通信), UART3 (Modbus), CAN1, I2C, ADC, RTC

### 2.2 Flash 内存布局

| 区域 | 起始地址 | 大小 | 扇区 | 用途 |
|------|---------|------|------|------|
| Bootloader | 0x08000000 | 32KB | 0, 1 | 启动管理，**不可 OTA** |
| APP1 (A区) | 0x08008000 | 224KB | 2, 3, 4, 5 | OTA A区固件 |
| APP2 (B区) | 0x08040000 | 224KB | 6, 7 | OTA B区固件 |
| DATA | 0x08060000 | 剩余 | 7 顶部 | 用户数据存储 |

> **注意 DATA 与 APP2 的地址重叠**: DATA 区起始地址 0x08060000 位于 APP2 的扇区 7 范围内 (0x08060000-0x0807FFFF)。如果 APP2 固件较大（接近 224KB），DATA 区会被覆盖。当前设计依赖 APP2 固件不占满扇区 7，DATA 使用扇区 7 的剩余空间。

### 2.3 三个固件程序的区别

| 特性 | APP1 (出厂) | APP2 (B区) | APP3 (A区OTA) |
|------|------------|-----------|---------------|
| 链接地址 | 0x08008000 | 0x08040000 | 0x08008000 |
| VTOR 设置方式 | 硬编码 0x08008000 | 动态判断 | 动态判断 |
| 写入运行标志地址 | APP1 末尾 | APP2 末尾 | APP1 末尾 |
| 看门狗(IWDG) | 默认关闭 | 启用 | 启用 |
| Task1 优先级 | 2 | 4 | 4 |
| Task2 功能 | 空闲循环 | 喂看门狗 (5s) | 喂看门狗 (5s) |
| OTA 暂停任务 | Task1,2,3 | Task1,3 (Task2 不暂停) | Task1,3 (Task2 不暂停) |

> **说明**: 
> - APP2 和 APP3 源码**大部分相同但有关键区别**: 写入运行标志时 APP2 写 `Application_2_Addr -4`，APP3 写 `Application_1_Addr -4`（因为 APP3 运行在 A 区位置，应激活 A 区标志）。其他文件（FreeRTOS_Demo、mqtt 等）相同。
> - 链接脚本也不同：APP2 链接到 0x08040000，APP3 链接到 0x08008000。
> - Bootloader 只能启动到 APP1 或 APP2 地址。APP3 是 APP1 的 OTA 替代品（编译到同一地址），APP3 通过动态 VTOR 可以跳转到任意区。

---

## 三、标志位系统详解

### 3.1 标志位位置

```
APP1 (0x08008000 + 0x38000 = 0x0803F800):
  [0x0803FFF8] = 看门狗恢复标志 (偏移 -8)
  [0x0803FFFC] = 运行标志       (偏移 -4)

APP2 (0x08040000 + 0x38000 = 0x08077FF8):
  [0x0807FFF8] = 看门狗恢复标志 (偏移 -8)
  [0x0807FFFC] = 运行标志       (偏移 -4)
```

### 3.2 标志位取值

| 标志 | 值 | 含义 |
|------|-----|------|
| 运行标志 (-4) | 0xFFFFFFFF | 擦除态/空 |
| 运行标志 (-4) | 0xAAAAAAAA | 正在运行/已激活 |
| 运行标志 (-4) | 0x00000000 | 已废弃（上次 OTA 时被旧固件标记） |
| 看门狗标志 (-8) | 0xFFFFFFFF | 正常（未触发恢复） |
| 看门狗标志 (-8) | 0x55555555 | Bootloader 写入，指示该 APP 应恢复运行 |

### 3.3 Flash 物理约束

**Flash 只能将 1 编程为 0，0 变回 1 必须扇区擦除。**

```
0xFFFFFFFF ──编程──→ 0xAAAAAAAA ──编程──→ 0x00000000
              (1→0)              (更多1→0)

0x00000000 ──× 无法写回 0xAAAAAAAA（0→1 需要擦除）
           ──× 无法写回 0xFFFFFFFF（0→1 需要擦除）
```

这是整个标志位系统设计的基础约束。擦除是恢复标志的唯一途径。

---

## 四、Bootloader 详细流程

### 4.1 入口

系统上电后，从 Flash 地址 `0x08000000` 开始执行，即 Bootloader。

### 4.2 Bootloader 主循环 (`Start_BootLoader`)

代码位置: `Bootloader/OTA_Bootloader/Core/Src/boot.c:191`

```
main() → 初始化时钟/GPIO/UART1 → while(1) { Start_BootLoader(); }
```

### 4.3 启动模式判断 (`Read_Start_Mode`)

代码位置: `Bootloader/OTA_Bootloader/Core/Src/boot.c:72`

判断优先级如下：

```
Step 1: 检查 IWDG 复位标志
  ├─ APP2 崩溃 (APP1=0x00000000, APP2=0xAAAAAAAA)
  │   → 写 0x55555555 到 APP1 看门狗标志 (-8)
  │   → 返回 Startup_1 (启动 APP1)
  │
  └─ APP1 崩溃 (APP1=0xAAAAAAAA, APP2=0x00000000)
      → 写 0x55555555 到 APP2 看门狗标志 (-8)
      → 返回 Startup_2 (启动 APP2)

Step 2: 检查运行标志 (-4)
  ├─ APP1 -4 == 0xAAAAAAAA → 返回 Startup_1
  └─ APP2 -4 == 0xAAAAAAAA → 返回 Startup_2

Step 3: OTA 完成判断
  ├─ 两个都是 0xFFFFFFFF → 返回 Startup_1 (首次出厂)
  ├─ APP1=0x00, APP2=0xAA → 返回 Startup_2 (OTA 后 APP2 运行)
  └─ APP1=0xAA, APP2=0x00 → 返回 Startup_1 (OTA 后 APP1 运行)

Step 4: 默认 → 返回 Startup_1
```

### 4.4 跳转执行 (`IAP_ExecuteApp`)

代码位置: `Bootloader/OTA_Bootloader/Core/Src/boot.c:161`

```c
void IAP_ExecuteApp(uint32_t App_Addr)
{
    // 1. 验证栈顶地址: 必须在 0x20000000 ~ 0x20020000 范围内
    if (((*(__IO uint32_t *)App_Addr) & 0x2FFD0000) == 0x20000000)
    {
        // 2. 去初始化 HAL 库
        HAL_DeInit();
        // 3. 关闭全局中断
        __disable_irq();
        // 4. 停止 SysTick
        SysTick->CTRL = 0;
        // 5. 设置主堆栈指针 MSP = 新APP的栈顶地址
        __set_MSP((*(__IO uint32_t *)App_Addr));
        // 6. 跳转到新APP的Reset_Handler (App_Addr + 4)
        JumpToApp = (Jump_Fun) *(__IO uint32_t *)(App_Addr + 4);
        JumpToApp();
    }
}
```

---

## 五、APP 启动流程

### 5.1 APP1 启动 (出厂程序)

代码位置: `APP1/Core/Src/main.c:86`

```
1. SCB->VTOR = Application_1_Addr (0x08008000)  ← 硬编码
2. __enable_irq()                                 ← 使能全局中断
3. HAL_Init()                                     ← HAL库初始化
4. SystemClock_Config()                           ← 时钟配置 168MHz
5. MX_GPIO/DMA/USART/I2C/TIM/RTC/ADC/CAN/CRC_Init()  ← 外设初始化
6. 使能 UART1/UART2 的 IDLE 中断 + DMA 接收
7. 传感器初始化 (AHT20, INA226)
8. 写入运行标志 (仅当 0xFFFFFFFF 时):
   写入 0xAAAAAAAA 到 APP1 区 -4 位置
9. FreeRTOS_Start() ← 启动任务调度器
```

### 5.2 APP2 启动 (动态 VTOR)

代码位置: `APP2/Core/Src/main.c:86`

```
1. 动态判断 VTOR (优先级递减):
   a. 读 APP1 -8 标志 → 0x55555555? → VTOR = APP1 (看门狗恢复)
   b. 读 APP2 -8 标志 → 0x55555555? → VTOR = APP2 (看门狗恢复)
   c. 读 APP1 -4 标志 → 0xAAAAAAAA? → VTOR = APP1
   d. 读 APP2 -4 标志 → 0xAAAAAAAA? → VTOR = APP2
   e. 默认 → VTOR = APP2

2-8. 与 APP1 相同的外设初始化流程

9. 写入运行标志 (仅当 0xFFFFFFFF 时):
   写入 0xAAAAAAAA 到 APP2 区 -4 位置

10. FreeRTOS_Start()
```

### 5.3 APP3 启动 (动态 VTOR, A 区 OTA 固件)

代码位置: `APP3/Core/Src/main.c:86`

与 APP2 的关键区别在于第 9 步写入运行标志的地址不同：

```
9. 写入运行标志 (仅当 0xFFFFFFFF 时):
   写入 0xAAAAAAAA 到 APP1 区 -4 位置
   ↑ APP3 链接在 0x08008000 (A区)，所以激活的是 APP1 区标志
```

> **注意**: IWDG 在 APP2/APP3 中启用 (`MX_IWDG_Init()`)，在 APP1 中默认注释掉。

---

## 六、通信协议与 MQTT 架构

### 6.1 物理层

```
STM32 UART2 (115200bps) ←→ 4G/WiFi 模块 (ML307 / ESP8266)
                                │
                          AT 指令 + MQTT
                                │
                          broker.emqx.io:1883
```

### 6.2 MQTT 主题设计

#### 下行主题 (服务器 → 设备)

| 宏定义 | 主题模式 | 用途 | QoS |
|--------|---------|------|-----|
| SUB1 | `v1/devices/me/rpc/request/+` | RPC 设备控制命令 | 1 |
| SUB2 | `v1/devices/me/attributes` | OTA 固件元信息 | 1 |
| SUB3 | `v2/fw/response/+/chunk/#` | OTA 固件分块数据 | 1 |

#### 上行主题 (设备 → 服务器)

| 宏定义 | 主题模式 | 用途 | QoS |
|--------|---------|------|-----|
| PUB1 | `v1/devices/me/telemetry` | 传感器遥测数据 / OTA 状态 | 1 |
| PUB2 | `v1/devices/me/rpc/response/` | RPC 命令回复 | 1 |
| PUB3 | `v2/fw/request/+/chunk/#` | 请求固件分块 | 1 |

### 6.3 MQTT 客户端 ID

使用 STM32 的 CPU 唯一 ID (`0x1FFF7A10` 开始的 96 位)，格式化为 24 字符十六进制字符串。

---

## 七、数据接收与解析链路（核心通信流程）

### 7.1 DMA + 中断接收机制

代码位置: `APP1/Core/Src/main.c:155-158`, `APP1/Core/Src/stm32f4xx_it.c:375-527`

```
配置阶段 (main.c):
  ① __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE)   ← 使能空闲中断
  ② HAL_UART_Receive_DMA(&huart2, RX2_Buf, 256)   ← 启动 DMA 循环接收

数据到达时:
  ③ DMA 自动将串口数据搬运到 RX2_Buf
  ④ DMA 半满中断 (HAL_UART_RxHalfCpltCallback):
     → Enqueue_Bytes(&QUART2, 前半段数据)
  ⑤ DMA 全满中断 (HAL_UART_RxCpltCallback):
     → Enqueue_Bytes(&QUART2, 后半段数据)
  ⑥ 串口空闲中断 (USER_UART_IRQHandler):
     → Enqueue_Bytes(&QUART2, 剩余数据)
     → 立即调用 WIFI4G_Parse_Queue(&QUART2)
```

**关键**: 数据先进入 FIFO 队列 `QUART2` (1024字节)，然后在 TIM5 的 10ms 中断或空闲中断中被取出解析。

### 7.2 队列解析 (`WIFI4G_Parse_Queue`)

代码位置: `APP1/Core/Src/wifi4g.c:39`

此函数在每个 TIM5 中断 (10ms) 和 UART2 空闲中断中被调用。

```
WIFI4G_Parse_Queue(QUART2):
  ① 将队列中所有字节取出到 RecvBuf[]
  ② 通过 UART1 转发收到的数据 (调试用途)
  ③ 判断 MQTTDownLoad_Flag:
     ├─ == 1 (MQTT 已连接):
     │   → CAT1_Parse_ATComander(RecvBuf)  ← 解析 MQTT 业务数据
     │   → return SET
     └─ == 0 (MQTT 未连接):
         → 在 RecvBuf 中查找 Parse_Substr (如 "OK\r\n" 或 "MQTT_CONNECT:")
         → 找到 → return WIFI4G_OK
         → 找到 "ERROR\r\n" → return WIFI4G_ERROR
         → 都没找到 → return WIFI4G_NOT
```

### 7.3 MQTT 消息解析 (`CAT1_Parse_ATComander`)

代码位置: `APP1/Core/Src/mqtt.c:375`

收到完整的 MQTT 数据包后，按主题特征进行匹配：

```
CAT1_Parse_ATComander(recvbuf):

  ┌─ 匹配 "v1/devices/me/rpc/request/"
  │   → 提取 JSON (花括号内容)
  │   → MQTT_Parse_DeviceData(json, requestId)
  │     解析 method 字段:
  │     ├─ "led1State/led1Status" → 控制/查询 LED1
  │     ├─ "beepState/beepStatus" → 控制/查询 BEEP
  │     ├─ "relayState/relayStatus" → 控制/查询 继电器
  │     └─ 通过 PUB2 主题回复状态
  │
  ├─ 匹配 "v1/devices/me/attributes"
  │   → 提取 JSON
  │   → MQTT_Parse_OTAData(json)
  │     解析字段:
  │     ├─ fw_title    → OTA_Info.fw_title
  │     ├─ fw_version  → OTA_Info.fw_version
  │     ├─ fw_size     → OTA_Info.fw_size
  │     ├─ fw_checksum → OTA_Info.fw_checksum
  │     └─ 设置 MQTT_OTA_Flag = 1  ← 触发 OTA 流程
  │
  └─ 匹配 "v2/fw/response/<id>/chunk/<n>,<bytes>,"
      → memcpy 固件数据到 OTA_Info.recv_buf
      → 设置 OTA_Info.recv_flag = 1  ← 通知下载循环收到数据
```

---

## 八、FreeRTOS 任务架构

### 8.1 任务列表

代码位置: `APP1/Core/Src/FreeRTOS_Demo.c`, `APP2/Core/Src/FreeRTOS_Demo.c`

**APP1 (出厂固件):**

| 任务 | 栈大小 | 优先级 | 功能 | 调度周期 |
|------|--------|--------|------|---------|
| Task_Start | 128 words | 1 | 创建子任务后自删除 | 一次性 |
| Task1 | 128 words | **2** | CANOpen 协议栈 | 500ms |
| Task2 | 128 words | 4 | 空闲循环 (无实质功能) | 500ms |
| Task3 | 256 words | 4 | Modbus 轮询 | 10ms |
| Task4 | 768 words | 4 | **MQTT连接 + OTA控制** | 500ms |

**APP2/APP3 (OTA 固件):**

| 任务 | 栈大小 | 优先级 | 功能 | 调度周期 |
|------|--------|--------|------|---------|
| Task_Start | 128 words | 1 | 创建子任务后自删除 | 一次性 |
| Task1 | 128 words | **4** | CANOpen 协议栈 | 500ms |
| Task2 | 128 words | 4 | **喂看门狗 (IWDG)** | 5s |
| Task3 | 256 words | 4 | Modbus 轮询 | 10ms |
| Task4 | 768 words | 4 | **MQTT连接 + OTA控制** | 500ms |

> **关键差异**: 
> - APP1 的 Task1 优先级为 2（低于其他任务），APP2/APP3 中所有任务优先级均为 4。
> - APP2/APP3 的 Task2 负责每 5 秒喂一次独立看门狗 (`HAL_IWDG_Refresh`)，**OTA 时不能暂停**此任务，否则下载期间会触发看门狗复位。

### 8.2 Task4 详细流程 (核心任务)

代码位置: `APP1/Core/Src/FreeRTOS_Demo.c:125`

```
void Task4(void * pvParameters)
{
    ① Queue_Init(&QUART2)                                    ← 初始化FIFO队列
    ② MQTT_Connect_MQTTServer()                              ← 连接MQTT服务器
       ├─ 成功: MQTTDownLoad_Flag = 1
       └─ 失败: 函数已内建重试
    ③ HAL_TIM_Base_Start_IT(&htim3)                          ← 启动TIM3 500ms周期中断
    ───────────────────────────────────────────────────

    while(1) {
        ④ 检查 MQTTUpLoad_flag:
           每隔 5s 触发一次 (TIM3 中断每 500ms 计数 10 次)
           → 上传遥测数据 (当前代码中注释掉)

        ⑤ 检查 MQTT_OTA_Flag:                               ← OTA 下载入口
           if (MQTT_OTA_Flag == 1) {
               MQTT_OTA_Flag = 0;
               暂停其他任务:
                 APP1: 暂停 Task1, Task2, Task3
                 APP2/APP3: 暂停 Task1, Task3 (Task2 喂狗，不暂停)
               MQTT_OTA_GetFW()                 ← 执行 OTA 下载 (阻塞)
               恢复已暂停的任务
           }

        ⑥ vTaskDelay(500)                                    ← 500ms 循环
    }
}
```

### 8.3 TIM3 中断的作用

代码位置: `APP1/Core/Src/main.c:286`

```
TIM3 中断 (每 500ms):
  ├─ 设置 TIM3_Timerout_Flag = 1
  └─ 计数器累加 10 次 (即 5 秒):
      → MQTTUpLoad_flag = 1  ← Task4 检测后上传数据
```

### 8.4 TIM5 中断的作用

代码位置: `APP1/Core/Src/main.c:295`

```
TIM5 中断 (每 10ms):
  → WIFI4G_CMD_Status = WIFI4G_Parse_Queue(&QUART2)
    (处理AT指令返回值和MQTT业务数据)
```

---

## 九、OTA 升级完整流程详解

### 9.1 触发条件

1. 设备已连接 MQTT 服务器 (`MQTTDownLoad_Flag == 1`)
2. 服务器通过 `v1/devices/me/attributes` 主题下发固件 JSON
3. `MQTT_Parse_OTAData()` 解析成功，设置 `MQTT_OTA_Flag = 1`
4. Task4 检测标志位，启动下载流程

### 9.2 `MQTT_OTA_GetFW()` 详细步骤

代码位置: `APP1/Core/Src/mqtt.c:670`

```
Step 1: 计算总块数
  size = OTA_Info.fw_size / 256
  if (fw_size % 256 != 0) size++

Step 2: 判断运行区，确定下载目标
  read APP1 -4:
    == 0xAAAAAAAA → 下载到 APP2 (0x08040000), 旧固件 = APP1
    ≠ 0xAAAAAAAA → read APP2 -4:
      == 0xAAAAAAAA → 下载到 APP1 (0x08008000), 旧固件 = APP2

Step 3: 擦除目标 Flash
  MyFlash_Erase(目标扇区)
  → 自动将目标区域的标志位重置为 0xFFFFFFFF

Step 4: 初始化 OTA 状态
  OTA_Info.received_bytes = 0
  OTA_Info.recv_flag = 0
  OTA_Info.request_id = HAL_GetTick() + (rand() & 0xFFFF)
  OTA_Info.chunk_id = 0
  OTA_Info.request_bytes = 256
  CRC32_Init(&crc_ctx)

Step 5: 逐块下载循环
  for i = 0; i < size; i++, chunk_id++:

    a. 如果最后一块不到 256 字节:
       OTA_Info.request_bytes = fw_size % 256

    b. 发送请求到 PUB3 主题:
       MQTT_SendData(PUB3, request_id, "256")
       → AT指令: MQPUB,1,v2/fw/request/<id>/chunk/<n>,256

    c. 等待服务器响应 (超时 10 秒):
       while (OTA_Info.recv_flag == 0) {
           if (--Timeout == 0):
               擦除目标区 → 返回失败
           HAL_Delay(1)
       }

    d. 清除接收标志: OTA_Info.recv_flag = 0

    e. 更新进度显示

    f. 写入 Flash:
       MyFlash_Save(目标地址 + i * 256, OTA_Info.recv_buf, 实际长度)

    g. 增量 CRC32:
       CRC32_Update(&crc_ctx, OTA_Info.recv_buf, 实际长度)

Step 6: CRC32 校验

  ┌─ CRC 一致 ─────────────────────────────────────
  │ ① 通过 PUB1 发送 {"fw_state":"UPDATED"}
  │ ② 旧固件 -4 写 0x00000000  (标记为废弃)
  │ ③ 新固件 -4 写 0xAAAAAAAA  (标记为激活)
  │    ** 顺序关键: 先废弃旧固件, 再激活新固件 **
  │    ** 防止中途断电导致两个 APP 都为 0xAAAAAAAA **
  │ ④ HAL_NVIC_SystemReset()  ← 系统复位
  │    → Bootloader 检测新固件标志 → 启动新固件
  │
  └─ CRC 不一致 ─────────────────────────────────
    ① 通过 PUB1 发送 {"fw_state":"FAILED"}
    ② 擦除目标区
    ③ MQTT_OTA_Flag = 1  (保持，允许重试)
    ④ return RESET
```

### 9.3 MyFlash 操作

代码位置: `APP1/Core/Src/flash.c`

```c
// 擦除操作
uint8_t MyFlash_Erase(uint32_t Sectors, uint32_t NbBytes) {
    if (Sectors == APP1_Flash_Bank)  // 扇区 2
        MyFLASH_Erase(2, 4, 0);     // 擦除扇区 2-5 (4个)
    else if (Sectors == APP2_Flash_Bank)  // 扇区 6
        MyFLASH_Erase(6, 2, 0);     // 擦除扇区 6-7 (2个)
}

// 写入操作 (逐字节)
uint8_t MyFlash_Save(uint32_t DataAdress, char *Buf, uint32_t msglen) {
    HAL_FLASH_Unlock();
    for (int i = 0; i < msglen; i++)
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, DataAdress + i, Buf[i]);
    HAL_FLASH_Lock();
}

// 读取操作
uint32_t MyFlash_ReadWord(uint32_t DataAdress) {
    return *((__IO uint32_t *)(DataAdress));
}
```

### 9.4 CRC32 校验

代码位置: `APP1/Core/Src/MyCRC.cpp`

- 使用软件 CRC32 (标准 IEEE 802.3 多项式: 0xEDB88320)
- 初始值: 0xFFFFFFFF
- 最终异或: 0xFFFFFFFF
- 字节序交换: 与服务器格式对齐
- 校验方式: `strncmp(本地crc32_hex_string, 服务器fw_checksum, strlen)`

---

## 十、看门狗恢复流程

### 10.1 正常运行时

```
APP 主循环中喂狗:
  HAL_IWDG_Refresh(&hiwdg)
```

### 10.2 崩溃恢复

```
APP2 运行中 → 崩溃 (未喂狗) → IWDG 复位
  │
  ▼
Bootloader:
  ① __HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) == SET
  ② __HAL_RCC_CLEAR_RESET_FLAGS()
  ③ 读取 APP1 -4 = 0x00000000 (被 OTA 废弃)
     读取 APP2 -4 = 0xAAAAAAAA (崩溃前在运行)
  ④ 判定: APP2 损坏, 需恢复到 APP1
  ⑤ 写 0x55555555 到 APP1 看门狗标志 (-8)
  ⑥ 启动 APP1
  │
  ▼
APP1 启动:
  ① 动态 VTOR 判断:
     读 APP1 -8 = 0x55555555 → VTOR = APP1
  ② APP1 -4 = 0x00000000 → Flash 约束无法写回 0xAAAAAAAA
  ③ 跳过写运行标志 (仅 0xFFFFFFFF 时写)
  ④ 正常运行恢复
  │
  ▼
用户修复问题后再次 OTA:
  擦除扇区 → APP1 -4 恢复为 0xFFFFFFFF
  → 下次启动 APP1 可以正常写 0xAAAAAAAA
  → 系统恢复正常乒乓升级
```

---

## 十一、乒乓升级完整生命周期

```
┌─────────────────────────────────────────────────────────────────────┐
│ 出厂状态                                                           │
│ APP1 (A区) = 0xFFFFFFFF (空)  APP2 (B区) = 0xFFFFFFFF (空)         │
│ Bootloader: 两个都为空 → 启动 APP1                                  │
│ APP1 启动: 写入 APP1 -4 = 0xAAAAAAAA (自己激活)                     │
│ → 状态: APP1(运行) = 0xAA    APP2(空) = 0xFF                       │
├─────────────────────────────────────────────────────────────────────┤
│ OTA 第一次: APP1 更新 APP2                                          │
│ ① 擦除 APP2 扇区 6-7 (→ APP2 -4 = 0xFF)                            │
│ ② 下载新固件到 APP2                                                │
│ ③ CRC 通过: APP1 -4 = 0x00, APP2 -4 = 0xAA, 复位                   │
│ → Bootloader: APP1=0x00 APP2=0xAA → 启动 APP2                      │
│ → 状态: APP1(废弃) = 0x00    APP2(运行) = 0xAA                      │
├─────────────────────────────────────────────────────────────────────┤
│ OTA 第二次: APP2 更新 APP1                                          │
│ ① 擦除 APP1 扇区 2-5 (→ APP1 -4 = 0xFF)                            │
│ ② 下载新固件到 APP1                                                │
│ ③ CRC 通过: APP2 -4 = 0x00, APP1 -4 = 0xAA, 复位                   │
│ → Bootloader: APP1=0xAA APP2=0x00 → 启动 APP1                      │
│ → 状态: APP1(运行) = 0xAA    APP2(废弃) = 0x00                      │
├─────────────────────────────────────────────────────────────────────┤
│ OTA 第三次: APP1 更新 APP2 ... 如此循环...                          │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 十二、完整时序图 (OTA 升级)

```
STM32设备                                           MQTT服务器 (broker.emqx.io)
   │                                                       │
   │  [上电 → Bootloader → 启动APP]                        │
   │                                                       │
   │──AT+MQMULTEN=1────────────────────────────────────→  │
   │──AT+MQSUBM 订阅 SUB1/SUB2/SUB3 ──────────────────→  │
   │──AT+REST (重启4G模块, 自动连接MQTT) ──────────────→  │
   │←────MQTT_CONNECT──────────────────────────────────  │
   │  [MQTTDownLoad_Flag = 1]                              │
   │                                                       │
   │  [TIM3 500ms中断 → 5秒计满 → MQTTUpLoad_flag = 1]    │
   │──PUB1: {TP,RH,VO,CU,PW,VR,CPU} ──────────────────→  │  (遥测数据, 代码中当前注释掉)
   │                                                       │
   │  [服务器发起 OTA 升级]                                 │
   │←──SUB2: {fw_title,fw_version,fw_size,fw_checksum}──  │  (固件元信息)
   │  [MQTT_Parse_OTAData → MQTT_OTA_Flag = 1]            │
   │                                                       │
   │  [Task4 检测到 OTA 标志]                              │
   │  [暂停 Task1, Task3 (APP2/3 不暂停 Task2-喂狗)]       │
   │  [擦除目标 Flash 区]                                   │
   │                                                       │
   │  ╔══════════════════════════════════════════╗        │
   │  ║ 逐块下载循环 (每个chunk 256字节)          ║        │
   │  ╠══════════════════════════════════════════╣        │
   │  ║                                          ║        │
   │  ║──PUB3: fw/request/<id>/chunk/0,256 ──→  ║        │
   │  ║←──SUB3: chunk 0 数据 ────────────────   ║        │
   │  ║  [recv_flag=1 → 写入Flash → CRC32更新]   ║        │
   │  ║                                          ║        │
   │  ║──PUB3: fw/request/<id>/chunk/1,256 ──→  ║        │
   │  ║←──SUB3: chunk 1 数据 ────────────────   ║        │
   │  ║  [recv_flag=1 → 写入Flash → CRC32更新]   ║        │
   │  ║                                          ║        │
   │  ║  ... 重复直到最后一个chunk ...            ║        │
   │  ║                                          ║        │
   │  ║  如果超时 10s → 擦除目标区 → 返回失败     ║        │
   │  ╚══════════════════════════════════════════╝        │
   │                                                       │
   │  [CRC32 校验]                                         │
   │                                                       │
   │  ┌─ 校验通过 ──────────────────────────────────────   │
   │  │ PUB1: {"fw_state":"UPDATED"} ──────────────→     │
   │  │ 旧固件 -4 = 0x00000000                            │
   │  │ 新固件 -4 = 0xAAAAAAAA                            │
   │  │ NVIC_SystemReset()                                │
   │  │                                                    │
   │  └─ 校验失败 ──────────────────────────────────────   │
   │     PUB1: {"fw_state":"FAILED"} ───────────────→     │
   │     擦除目标区                                        │
   │                                                       │
   ▼  [系统复位]                                           ▼
```

---

## 十三、关键设计决策与注意事项

### 13.1 为什么先废弃旧固件再激活新固件

```c
// 正确的顺序 (代码中已实现):
MyFlash_Save(旧固件地址 - 4, 0x00000000);   // ① 先废弃
MyFlash_Save(新固件地址 - 4, 0xAAAAAAAA);   // ② 再激活

// 如果顺序颠倒, 在①②之间断电:
// → 两个 APP 都是 0xAAAAAAAA → Bootloader 无法判断 → 系统混乱
```

### 13.2 为什么看门狗恢复后不写运行标志

Flash 约束: `0x00000000 → 0xAAAAAAAA` 需要擦除扇区（会破坏固件本身）。所以看门狗恢复时，APP 的 -4 标志仍然是 `0x00000000`。这意味着：
- 该 APP 无法再通过 `0xFFFFFFFF → 0xAAAAAAAA` 的路径激活自己
- 只有等待下一次 OTA 擦除该扇区，标志被重置为 `0xFFFFFFFF`
- Bootloader 在检测到看门狗复位标志后，使用 -8 标志来处理恢复启动

### 13.3 为什么 OTA 时暂停其他任务

**APP1** (代码位置: `APP1/Core/Src/FreeRTOS_Demo.c:147-149`):

```c
vTaskSuspend(Task1_Handle);  // 暂停 CANOpen
vTaskSuspend(Task2_Handle);  // 暂停空闲任务 (APP1 无喂狗)
vTaskSuspend(Task3_Handle);  // 暂停 Modbus (10ms 轮询)
```

**APP2/APP3** (代码位置: `APP2/Core/Src/FreeRTOS_Demo.c:146-147`):

```c
vTaskSuspend(Task1_Handle);  // 暂停 CANOpen
// Task2 不暂停! 因为它负责喂狗 (HAL_IWDG_Refresh)
vTaskSuspend(Task3_Handle);  // 暂停 Modbus (10ms 轮询)
```

原因:
1. Flash 写入/擦除期间 CPU 停顿，不能处理中断任务
2. 避免 Modbus/CAN 通信超时触发异常
3. 确保 OTA 下载过程不受其他任务干扰
4. **APP2/APP3 中 Task2 必须保持运行**：每 5 秒喂一次 IWDG，如果暂停会导致下载期间看门狗复位，OTA 失败

### 13.4 Flash 擦除时必须固定扇区数

代码位置: `APP1/Core/Src/flash.c:79`

```c
// APP1 总是擦除 4 个扇区 (2-5)
// APP2 总是擦除 2 个扇区 (6-7)
// 即使固件很小也必须擦除这么多, 因为:
// - 标志位在最后 8 字节
// - 必须把整个 APP 区域的旧代码清干净
// - 否则残留的旧代码可能导致 Flash 写入时失败
```

### 13.5 数据缓冲大小

- DMA 缓冲区: `DMA_BUF_SIZE = 256` 字节
- FIFO 队列: `NSize = 1024` 字节 (APP1/APP2 保持一致)
- Task4 栈: 768 words (最大任务栈，因为 OTA 下载循环在 Task4 上下文中执行)

---

## 十四、关键代码位置索引

| 功能 | 文件路径 | 函数/变量 |
|------|---------|----------|
| Bootloader 入口 | `Bootloader/.../main.c:86` | `main()` |
| 启动模式判断 | `Bootloader/.../boot.c:72` | `Read_Start_Mode()` |
| APP 跳转 | `Bootloader/.../boot.c:161` | `IAP_ExecuteApp()` |
| APP1 启动 | `APP1/Core/Src/main.c:86` | `main()` |
| APP2/APP3 启动 | `APP2/Core/Src/main.c:86` | `main()` |
| FreeRTOS 任务创建 | `APP1/Core/Src/FreeRTOS_Demo.c:45` | `FreeRTOS_Start()` |
| Task4 (MQTT/OTA) | `APP1/Core/Src/FreeRTOS_Demo.c:125` | `Task4()` |
| MQTT 连接 | `APP1/Core/Src/mqtt.c:203` | `MQTT_Connect_MQTTServer()` |
| AT 指令检查 | `APP1/Core/Src/wifi4g.c:25` | `Test_WIFI4G_CMD_Status()` |
| 队列解析 | `APP1/Core/Src/wifi4g.c:39` | `WIFI4G_Parse_Queue()` |
| MQTT 消息分发 | `APP1/Core/Src/mqtt.c:375` | `CAT1_Parse_ATComander()` |
| OTA 信息解析 | `APP1/Core/Src/mqtt.c:611` | `MQTT_Parse_OTAData()` |
| RPC 设备控制 | `APP1/Core/Src/mqtt.c:442` | `MQTT_Parse_DeviceData()` |
| OTA 下载主循环 | `APP1/Core/Src/mqtt.c:670` | `MQTT_OTA_GetFW()` |
| MQTT 数据发送 | `APP1/Core/Src/mqtt.c:554` | `MQTT_SendData()` |
| Flash 擦除 | `APP1/Core/Src/flash.c:79` | `MyFlash_Erase()` |
| Flash 写入 | `APP1/Core/Src/flash.c:100` | `MyFlash_Save()` |
| Flash 读取 | `APP1/Core/Src/flash.c:13` | `MyFlash_ReadWord()` |
| CRC32 计算 | `APP1/Core/Src/MyCRC.cpp:13` | `CRC32_Init/Update/Final()` |
| DMA/UART 中断 | `APP1/Core/Src/stm32f4xx_it.c:398` | `HAL_UART_RxCpltCallback()` |
| 空闲中断处理 | `APP1/Core/Src/stm32f4xx_it.c:464` | `USER_UART_IRQHandler()` |
| TIM3/TIM5 回调 | `APP1/Core/Src/main.c:276` | `HAL_TIM_PeriodElapsedCallback()` |
| 标志位定义 | `APP1/Core/Inc/wifi4g.h` | `Application_Size`, MQTT主题 |
| OTA 结构体 | `APP1/Core/Inc/mqtt.h:14` | `OTA_FW_Info_T` |
| Flash 扇区定义 | `APP1/Core/Inc/mqtt.h:11` | `APP1_Flash_Bank`/`APP2_Flash_Bank` |
| 内存地址定义 | `Bootloader/.../boot.h:14` | `Application_1_Addr`/`Application_2_Addr` |

---

> **文档版本**: v1.0
> **编写日期**: 2026-07-23
> **工程路径**: `D:\studet2\OTA\OTA`
