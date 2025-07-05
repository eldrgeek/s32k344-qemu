/*
 * NXP S32K344 - ARM Cortex-M7 Automotive Microcontroller
 *
 * Copyright (c) 2025 QEMU S32K344 Project
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#ifndef HW_ARM_S32K344_H
#define HW_ARM_S32K344_H

#include "hw/boards.h"
#include "hw/arm/armv7m.h"
#include "hw/char/lpuart.h"
#include "hw/timer/systick.h"
#include "hw/misc/s32k344_siul2.h"
#include "qom/object.h"

/* S32K344 Memory Layout */
#define S32K344_FLASH_BASE      0x00400000
#define S32K344_FLASH_SIZE      (4 * 1024 * 1024)  /* 4MB */
#define S32K344_SRAM_BASE       0x20000000
#define S32K344_SRAM_SIZE       (320 * 1024)       /* 320KB */
#define S32K344_DTCM_BASE       0x20000000
#define S32K344_DTCM_SIZE       (64 * 1024)        /* 64KB */
#define S32K344_ITCM_BASE       0x00000000
#define S32K344_ITCM_SIZE       (64 * 1024)        /* 64KB */

/* System Integration Unit Lite 2 (SIUL2) */
#define S32K344_SIUL2_BASE      0x40290000
#define S32K344_SIUL2_SIZE      0x10000

/* Low Power Universal Asynchronous Receiver/Transmitter (LPUART) */
#define S32K344_LPUART0_BASE    0x40328000
#define S32K344_LPUART1_BASE    0x4032C000
#define S32K344_LPUART2_BASE    0x40330000
#define S32K344_LPUART3_BASE    0x40334000
#define S32K344_LPUART4_BASE    0x40338000
#define S32K344_LPUART5_BASE    0x4033C000
#define S32K344_LPUART6_BASE    0x40340000
#define S32K344_LPUART7_BASE    0x40344000
#define S32K344_LPUART8_BASE    0x40348000
#define S32K344_LPUART9_BASE    0x4034C000
#define S32K344_LPUART10_BASE   0x40350000
#define S32K344_LPUART11_BASE   0x40354000
#define S32K344_LPUART12_BASE   0x40358000
#define S32K344_LPUART13_BASE   0x4035C000
#define S32K344_LPUART14_BASE   0x40360000
#define S32K344_LPUART15_BASE   0x40364000

/* System Timer (SysTick) */
#define S32K344_SYSTICK_BASE    0xE000E010

/* Nested Vectored Interrupt Controller (NVIC) */
#define S32K344_NVIC_BASE       0xE000E100
#define S32K344_NVIC_SIZE       0x1000

/* FlexCAN */
#define S32K344_FLEXCAN0_BASE   0x40304000
#define S32K344_FLEXCAN1_BASE   0x40308000
#define S32K344_FLEXCAN2_BASE   0x4030C000
#define S32K344_FLEXCAN3_BASE   0x40310000
#define S32K344_FLEXCAN4_BASE   0x40314000
#define S32K344_FLEXCAN5_BASE   0x40318000

/* IRQ Numbers */
#define S32K344_IRQ_LPUART0     32
#define S32K344_IRQ_LPUART1     33
#define S32K344_IRQ_LPUART2     34
#define S32K344_IRQ_LPUART3     35
#define S32K344_IRQ_LPUART4     36
#define S32K344_IRQ_LPUART5     37
#define S32K344_IRQ_LPUART6     38
#define S32K344_IRQ_LPUART7     39
#define S32K344_IRQ_LPUART8     40
#define S32K344_IRQ_LPUART9     41
#define S32K344_IRQ_LPUART10    42
#define S32K344_IRQ_LPUART11    43
#define S32K344_IRQ_LPUART12    44
#define S32K344_IRQ_LPUART13    45
#define S32K344_IRQ_LPUART14    46
#define S32K344_IRQ_LPUART15    47

#define S32K344_IRQ_FLEXCAN0_0  48
#define S32K344_IRQ_FLEXCAN0_1  49
#define S32K344_IRQ_FLEXCAN0_2  50
#define S32K344_IRQ_FLEXCAN0_3  51

/* CPU Core */
#define S32K344_CPU_TYPE        ARM_CPU_TYPE_NAME("cortex-m7")
#define S32K344_CPU_FREQ_HZ     (160 * 1000 * 1000)  /* 160MHz */

/* Board Configuration */
#define S32K344_BOARD_NAME      "s32k344"
#define S32K344_BOARD_DESC      "NXP S32K344 ARM Cortex-M7 Automotive MCU"

/* Machine State */
#define TYPE_S32K344_MACHINE    "s32k344"
OBJECT_DECLARE_SIMPLE_TYPE(S32K344MachineState, S32K344_MACHINE)

struct S32K344MachineState {
    /*< private >*/
    MachineState parent_obj;
    
    /*< public >*/
    ARMv7MState armv7m;
    
    /* Peripheral devices */
    DeviceState *siul2;         /* GPIO/SIUL2 */
    DeviceState *lpuart[16];    /* LPUART instances */
    DeviceState *flexcan[6];    /* FlexCAN instances */
    
    /* Clock and reset */
    uint32_t sysclk_freq;
    
    /* Boot configuration */
    bool secure_boot;
    uint32_t boot_mode;
};

/* Machine initialization */
void s32k344_machine_init(MachineState *machine);

/* Board-specific helpers */
static inline hwaddr s32k344_get_flash_base(void)
{
    return S32K344_FLASH_BASE;
}

static inline hwaddr s32k344_get_sram_base(void)
{
    return S32K344_SRAM_BASE;
}

static inline uint32_t s32k344_get_cpu_freq(void)
{
    return S32K344_CPU_FREQ_HZ;
}

#endif /* HW_ARM_S32K344_H */ 