/*
******************************************************************************
**

**  File        : LinkerScript.ld
**
**  Author		: STM32CubeMX
**
**  Abstract    : Linker script for STM32F407VETx series
**                512Kbytes FLASH and 192Kbytes RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**  Distribution: The file is distributed “as is,” without any warranty
**                of any kind.
**
*****************************************************************************
** @attention
**
** <h2><center>&copy; COPYRIGHT(c) 2025 STMicroelectronics</center></h2>
**
** Redistribution and use in source and binary forms, with or without modification,
** are permitted provided that the following conditions are met:
**   1. Redistributions of source code must retain the above copyright notice,
**      this list of conditions and the following disclaimer.
**   2. Redistributions in binary form must reproduce the above copyright notice,
**      this list of conditions and the following disclaimer in the documentation
**      and/or other materials provided with the distribution.
**   3. Neither the name of STMicroelectronics nor the names of its contributors
**      may be used to endorse or promote products derived from this software
**      without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
** CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
** OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
** 链接脚本告诉链接器：芯片的内存有多大、代码和数据分别放哪里、栈和堆留多少空间。最终生成 .elf 文件时可被烧录到 STM32 的 FLASH。
**
    **---
  **  逐段解释
**
  **  1. ENTRY (第 53 行)
**
  **  ENTRY(Reset_Handler)
**
  **  指定程序入口点：上电后第一个执行的函数是 Reset_Handler（在启动文件里定义）。
**
  **  2. MEMORY — 定义芯片存储区域 (第 56-61 行)
**
    **RAM    : ORIGIN = 0x20000000, LENGTH = 128K
    **CCMRAM : ORIGIN = 0x10000000, LENGTH = 64K
  **  FLASH  : ORIGIN = 0x08000000, LENGTH = 512K
**
  **  告诉链接器芯片有三块物理存储，地址和大小各是多少。链接器会确保任何段都不会超出你声明的大小。
**
  ****  3. 栈和堆的最小值 (第 64-68 行)
**
   ** _e**stack = ORIGIN(RAM) + LENGTH(RAM);   // 栈顶 = RAM末尾 (0x20000000 + 128K)
*_Min_Heap_Size = 0x400;       required amount of heap
*_Min_Stack_Size = 0x800; required amount of stack
**
    **- _estack 在启动代码里会被写进 MSP（主栈指针），栈从 RAM 顶部向下增长。
 *
  **  - _Min_Heap_Size / _Min_Stack_Size 两个值只是保底检查：如果代码里的 bss + data 太大导致剩下的空间不够这 3KB，链接器会报错。
**
  **  4. SECTIONS — 各段怎么放 (第 71 行开始)
**
    **.isr_vector (第 74-79 行)
  **  中断向量表，必须放在 FLASH 最开头。STM32 上电先读这里的栈指针和复位向量。
**
    **.text (第 82-96 行)
  **  所有函数编译出来的机器码。.text* 是通配，把库文件的代码段也收进来。_etext 标记代码段结束位置。
**
   ** .rodata (第 99-105 行)
  ****  只读数据 — 你代码里的字符串常量、const 全局变量等。
**
    **.ARM.extab / .ARM (第 107-121 行)
  **  ARM 异常处理相关的表（展开栈帧用的），C++ 异常或 backtrace 会用到，STM32 裸机基本没用但留着无害。
**
    **.preinit_array / .init_array / .fini_array (第 123-150 行)
  **  C++ 全局对象的构造/析构函数指针数组。在 main() 之前逐个调用初始化，程序结束时调析构。
**
    **.ccmram (第 160-169 行)
  **  CCMRAM 段。如果你有变量声明了 __attribute__((section(".ccmram")))，就放到这块 64KB 的紧耦合内存里（CPU 访问比普通 RAM 快，但 DMA 访问不了）。
**
  **  >CCMRAM AT> FLASH：运行地址在 CCMRAM，但初始值烧在 FLASH 里，启动代码负责拷贝过去。
**
    **.data (第 175-185 行)
  **  已初始化的全局变量。比如 int x = 5;。初始值存在 FLASH 里，启动时 _sidata 到 _sdata 这一段数据被拷贝到 RAM。
**
  **  _sidata = data 在 FLASH 中的加载地址，_sdata = 在 RAM 中的运行地址。
**
    **.tdata / .tbss (第 188-231 行)
  **  TLS（线程局部存储）相关。FreeRTOS 任务里如果用了 __thread 变量会用到。启动代码需要处理这部分的拷贝和清零。
**
  **  .bss (第 233-243 行)
**    未初始化（或初始化为 0）的全局变量。在 RAM 里但不占 FLASH 空间。启动代码负责把这片内存清零。_sbss / _ebss 给启动代码做边界。
**
 **   ._user_heap_stack (第 249-258 行)
 **   不实际分配空间，只是做个检查——确保 data + bss + heap + stack 加起来不超过 RAM 总大小。超了链接器就报错。
**
 **   /DISCARD/ (第 263-268 行)
 **   扔掉标准库的 libc/libm/libgcc 里不需要的段，减小最终固件体积。
**
**    ---
**    关键符号总结（启动代码会引用）

**    ┌─────────────────────┬──────────────────────────┐
**    │        符号         │           用途           │
**    ├─────────────────────┼──────────────────────────┤
**    │ _estack             │ 栈顶地址 → MSP           │
**    ├─────────────────────┼──────────────────────────┤
**    │ _sidata → _sdata    │ FLASH → RAM 拷贝 data 段 │
**    ├─────────────────────┼──────────────────────────┤
**    │ _sbss → _ebss       │ 清零 bss 段              │
**    ├─────────────────────┼──────────────────────────┤
**    │ _sccmram → _eccmram │ 初始化 CCMRAM（如有）    │
**    └─────────────────────┴──────────────────────────┘
**
**    简单说：MEMORY 画地盘，SECTIONS 摆家具，启动代码按符号搬家
*****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)   /*53*/

/* Specify the memory areas */
MEMORY
{
RAM (xrw)      : ORIGIN =   0x20000000, LENGTH = 128K
CCMRAM (xrw)      : ORIGIN =0x10000000, LENGTH = 64K
FLASH (rx)      : ORIGIN =  0x08008000, LENGTH = 224K
}

/* Highest address of the user mode stack */
_estack = ORIGIN(RAM) + LENGTH(RAM);    /* end of RAM */
_sstack = _estack - _Min_Stack_Size;
/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x400;      /* required amount of heap  */
_Min_Stack_Size = 0x800; /* required amount of stack */

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  .ARM.extab (READONLY) : /* The "READONLY" keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    . = ALIGN(4);
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(4);
  } >FLASH

  .ARM (READONLY) : /* The "READONLY" keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    . = ALIGN(4);
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
    . = ALIGN(4);
  } >FLASH

  .preinit_array (READONLY) : /* The "READONLY" keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .init_array (READONLY) : /* The "READONLY" keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .fini_array (READONLY) : /* The "READONLY" keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(4);
  } >FLASH

  _siccmram = LOADADDR(.ccmram);

  /* CCM-RAM section
  *__attribute__((section(".ccmram")))
  * IMPORTANT NOTE!
  * If initialized variables will be placed in this section,
  * the startup code needs to be modified to copy the init-values.
  */
  .ccmram :
  {
    . = ALIGN(4);
    _sccmram = .;       /* create a global symbol at ccmram start */
    *(.ccmram)
    *(.ccmram*)

    . = ALIGN(4);
    _eccmram = .;       /* create a global symbol at ccmram end */
  } >CCMRAM AT> FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
    *(.RamFunc)        /* .RamFunc sections */
    *(.RamFunc*)       /* .RamFunc* sections */

    . = ALIGN(4);
  } >RAM AT> FLASH

 /* Initialized TLS data section */
  .tdata : ALIGN(4)
  {
    *(.tdata .tdata.* .gnu.linkonce.td.*)
    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
    PROVIDE(__data_end = .);
    PROVIDE(__tdata_end = .);
  } >RAM AT> FLASH

  PROVIDE( __tdata_start = ADDR(.tdata) );
  PROVIDE( __tdata_size = __tdata_end - __tdata_start );

  PROVIDE( __data_start = ADDR(.data) );
  PROVIDE( __data_size = __data_end - __data_start );

  PROVIDE( __tdata_source = LOADADDR(.tdata) );
  PROVIDE( __tdata_source_end = LOADADDR(.tdata) + SIZEOF(.tdata) );
  PROVIDE( __tdata_source_size = __tdata_source_end - __tdata_source );

  PROVIDE( __data_source = LOADADDR(.data) );
  PROVIDE( __data_source_end = __tdata_source_end );
  PROVIDE( __data_source_size = __data_source_end - __data_source );
  /* Uninitialized data section */
  .tbss (NOLOAD) : ALIGN(4)
  {
     /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.tbss .tbss.*)
    . = ALIGN(4);
    PROVIDE( __tbss_end = . );
  } >RAM

  PROVIDE( __tbss_start = ADDR(.tbss) );
  PROVIDE( __tbss_size = __tbss_end - __tbss_start );
  PROVIDE( __tbss_offset = ADDR(.tbss) - ADDR(.tdata) );

  PROVIDE( __tls_base = __tdata_start );
  PROVIDE( __tls_end = __tbss_end );
  PROVIDE( __tls_size = __tls_end - __tls_base );
  PROVIDE( __tls_align = MAX(ALIGNOF(.tdata), ALIGNOF(.tbss)) );
  PROVIDE( __tls_size_align = (__tls_size + __tls_align - 1) & ~(__tls_align - 1) );
  PROVIDE( __arm32_tls_tcb_offset = MAX(8, __tls_align) );
  PROVIDE( __arm64_tls_tcb_offset = MAX(16, __tls_align) );

  .bss (NOLOAD) : ALIGN(4)
  {
    *(.bss)
    *(.bss*)
    *(COMMON)

      . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
      PROVIDE( __bss_end = .);
  } >RAM
  PROVIDE( __non_tls_bss_start = ADDR(.bss) );

  PROVIDE( __bss_start = __tbss_start );
  PROVIDE( __bss_size = __bss_end - __bss_start );

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack (NOLOAD) :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM



  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a:* ( * )
    libm.a:* ( * )
    libgcc.a:* ( * )
  }

}
