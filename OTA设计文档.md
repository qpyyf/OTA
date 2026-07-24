# STM32F407 双区 OTA 设计方案

## 1. Flash 内存布局

| 区域 | 起始地址 | 大小 | 扇区 | 用途 |
|------|---------|------|------|------|
| Bootloader | 0x08000000 | 32KB | 0, 1 | 启动管理，不可 OTA |
| APP1 (出厂) | 0x08008000 | 224KB | 2, 3, 4, 5 | 出厂固件 |
| APP2 (OTA) | 0x08040000 | 224KB | 6, 7 | OTA 更新固件 |
| DATA | 0x08060000 | 剩余 | 7 顶部 | 用户数据存储 |

- Bootloader 不可被 OTA 更新，确保永远有恢复能力
- APP1 和 APP2 各 224KB，交替作为"运行区"和"更新目标区"
- 两个 APP 源码相同，但链接脚本不同（APP1 链接到 0x08008000，APP2 链接到 0x08040000）

## 2. 标志位设计

每个 APP 区域的最后 8 字节存放两个 32 位标志：

| 偏移 | 名称 | 可能值 | 含义 |
|------|------|--------|------|
| `Addr + Size - 4` | 运行标志 | 0xFFFFFFFF | 擦除态/空 |
| | | 0xAAAAAAAA | 正在运行/已激活 |
| | | 0x00000000 | 已废弃（上次 OTA 时被标记） |
| `Addr + Size - 8` | 看门狗恢复标志 | 0xFFFFFFFF | 正常（未触发恢复） |
| | | 0x55555555 | Bootloader 写入，指示该 APP 应恢复运行 |

### Flash 物理约束（重要！）

**Flash 只能将 1 编程为 0，0 变回 1 必须扇区擦除。**

所以状态转换只能是单方向的（除擦除外）：

```
0xFFFFFFFF ──编程──→ 0xAAAAAAAA ──编程──→ 0x00000000
                (1→0)              (更多1→0)

0x00000000 ──× 无法写回 0xAAAAAAAA（0→1 需要擦除）
           ──× 无法写回 0xFFFFFFFF（0→1 需要擦除）
```

**关键结论：擦除是恢复标志的唯一途径。** OTA 流程中 `MyFlash_Erase()` 擦除目标扇区时，会把标志一起重置为 0xFFFFFFFF。

## 3. 固件编译

- **APP1**: 链接脚本 `FLASH ORIGIN = 0x08008000`，VTOR 硬编码 `Application_1_Addr`
- **APP2**: 链接脚本 `FLASH ORIGIN = 0x08040000`，VTOR 动态判断（见第 5 节）
- 两个 APP 必须分别编译，不可混用

## 4. Bootloader 启动流程

```
上电复位
  │
  ├─ 读 APP1 运行标志(-4)
  ├─ 读 APP2 运行标志(-4)
  ├─ 读 APP1 看门狗标志(-8)
  ├─ 读 APP2 看门狗标志(-8)
  │
  ├─ APP1 -4 = 0xAAAAAAAA → 启动 APP1
  ├─ APP2 -4 = 0xAAAAAAAA → 启动 APP2
  ├─ 两者都 = 0xFFFFFFFF → 启动 APP1（出厂首次启动）
  │
  ├─ 检测到 IWDG 复位:
  │   ├─ APP1 损坏(APP1=0x00000000, APP2=0xAAAAAAAA)
  │   │   → 写 0x55555555 到 APP1 -8, 启动 APP1
  │   └─ APP2 损坏(APP1=0xAAAAAAAA, APP2=0x00000000)
  │       → 写 0x55555555 到 APP2 -8, 启动 APP2
  │
  ├─ APP1=0x00000000, APP2=0xAAAAAAAA → 启动 APP2（OTA 刚更新 APP2）
  ├─ APP1=0xAAAAAAAA, APP2=0x00000000 → 启动 APP1（OTA 刚更新 APP1）
  └─ 默认 → 启动 APP1
```

## 5. APP2 VTOR 动态判断

APP2 在 main() 中设置 VTOR 时按以下优先级判断：

```
读 -8 看门狗标志:
  ├─ APP1 -8 = 0x55555555 → VTOR = APP1_Addr (看门狗恢复到 APP1)
  ├─ APP2 -8 = 0x55555555 → VTOR = APP2_Addr (看门狗恢复到 APP2)
  └─ 都不是 → 读 -4 运行标志:
       ├─ APP1 -4 = 0xAAAAAAAA → VTOR = APP1_Addr
       ├─ APP2 -4 = 0xAAAAAAAA → VTOR = APP2_Addr
       └─ 都不是 → VTOR = APP2_Addr (默认)
```

## 6. OTA 升级流程（以 APP1 → APP2 为例）

```
1. APP1 收到固件信息(标题, 版本, 大小, CRC32)
   
2. 判断谁在运行:
   读 APP1 -4 = 0xAAAAAAAA → APP1 在运行
   → 新固件写入 APP2(0x08040000)，旧固件是 APP1(0x08008000)
   
3. 擦除目标区:
   MyFlash_Erase(APP2_Flash_Bank, fw_size)
   → 擦除扇区 6-7，APP2 -4 变为 0xFFFFFFFF
   
4. 逐块下载(每块 256 字节):
   for chunk_id = 0 to total_chunks:
       发送 MQTT 请求 → 等待响应 → 写入 Flash → CRC32 增量计算
       超时则擦除目标区，返回失败
       
5. CRC32 校验:
   本地 CRC vs 服务器 CRC
   
6. 校验通过:
   ① 旧固件(APP1) -4 写 0x00000000  [0xAAAAAAAA → 0x00000000 ✓]
   ② 新固件(APP2) -4 写 0xAAAAAAAA  [0xFFFFFFFF → 0xAAAAAAAA ✓]
   ③ NVIC_SystemReset()
   
7. 校验失败:
   擦除目标区，返回失败，MQTT_OTA_Flag 保持 1
```

**特点：** `MQTT_OTA_GetFW()` 自动判断谁在运行并决定写入目标。APP1 运行时下载到 APP2，APP2 运行时下载到 APP1（乒乓升级）。

## 7. 看门狗恢复流程

```
APP2 运行中 → 崩溃(未喂狗) → IWDG 复位
  │
  ▼
Bootloader 检测到 IWDG 复位标志:
  APP2 -4 = 0xAAAAAAAA (崩溃前在运行)
  APP1 -4 = 0x00000000 (之前 OTA 被废弃)
  → 判定 APP2 损坏
  → 写 0x55555555 到 APP1 -8
  → 启动 APP1
  │
  ▼
APP1 启动:
  读 APP1 -8 = 0x55555555 → VTOR = APP1_Addr
  APP1 -4 = 0x00000000 → 无法写 0xAAAAAAAA (Flash 约束)
  → 不写运行标志，直接进入正常任务
  → APP1 恢复运行
  │
  ▼
用户修复问题后，再次 OTA:
  → APP1 下载新固件到 APP2(期间擦除扇区 6-7)
  → APP2 -4 恢复为 0xFFFFFFFF → 写入 0xAAAAAAAA
  → 系统恢复正常乒乓升级
```

## 8. 正常复位流程

```
APP2 运行中 → 断电重启/外部复位

Bootloader:
  APP1 -4 = 0x00000000
  APP2 -4 = 0xAAAAAAAA
  → 启动 APP2

APP2:
  读 -8: 无 0x55555555
  读 -4: APP2 = 0xAAAAAAAA
  → VTOR = APP2_Addr
  → APP2 -4 已经是 0xAAAAAAAA，跳过写入
  → APP2 继续运行
```

## 9. 乒乓升级完整生命周期

```
出厂:         APP1(运行)  APP2(空)
              ─────────
              APP1 写自己 -4 = 0xAAAAAAAA

OTA 1:        APP1 → 更新 APP2
              APP1 -4 = 0x00000000  APP2 -4 = 0xAAAAAAAA
              ─────────             ─────────
              复位 → Bootloader 启动 APP2

OTA 2:        APP2 → 更新 APP1
              擦除扇区 2-5(含 APP1 -4)
              APP2 -4 = 0x00000000  APP1 -4 = 0xAAAAAAAA
                                    ─────────
              复位 → Bootloader 启动 APP1

OTA 3:        APP1 → 更新 APP2
              擦除扇区 6-7(含 APP2 -4)
              ...循环往复...
```

## 10. APP2 工程配置清单

| 文件 | 配置项 | 值 | 说明 |
|------|--------|-----|------|
| `STM32F407XX_FLASH.ld` | FLASH ORIGIN | 0x08040000 | **关键！** 不能和 APP1 相同 |
| `Core/Inc/main.h` | Application_2_Addr | 0x08040000U | 地址宏定义 |
| `Core/Src/main.c` | VTOR 设置 | 动态判断(见第5节) | 不在库文件里改 |
| `Core/Src/main.c` | 运行标志写入 | 仅 0xFFFFFFFF 时写 | 防止写 0x00000000 |
| `Core/Inc/fifo.h` | NSize | 1024 | 与 APP1 同步 |

## 11. 注意事项

1. **两个固件必须分别编译。** APP1 和 APP2 的 .elf 文件不同，OTA 服务器需要下发正确的固件
2. **先擦除再写新标志。** OTA 的 `MyFlash_Erase()` 是关键步骤，它把所有废弃标志重置为 0xFFFFFFFF
3. **APP1 不改动。** APP1 保持硬编码 VTOR，只改 APP2
4. **看门狗恢复后不写运行标志。** 旧固件的 -4 已是 0x00000000，写不了。Bootloader 通过 -8 标志和自身逻辑处理恢复
