/*
 * NXP S32K344 - ARM Cortex-M7 Automotive Microcontroller Machine
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

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/boards.h"
#include "hw/qdev-properties.h"
#include "hw/qdev-clock.h"
#include "hw/arm/armv7m.h"
#include "hw/arm/boot.h"
#include "sysemu/sysemu.h"
#include "qemu/log.h"
#include "qemu/error-report.h"
#include "s32k344.h"

/* S32K344 Memory Layout */
#define S32K344_FLASH_BASE      0x00400000  /* Program Flash - 4MB */
#define S32K344_FLASH_SIZE      0x00400000  /* 4MB */
#define S32K344_SRAM_BASE       0x20000000  /* System RAM - 512KB */
#define S32K344_SRAM_SIZE       0x00080000  /* 512KB */
#define S32K344_DTCM_BASE       0x20400000  /* Data TCM - 64KB */
#define S32K344_DTCM_SIZE       0x00010000  /* 64KB */
#define S32K344_ITCM_BASE       0x00000000  /* Instruction TCM - 64KB */
#define S32K344_ITCM_SIZE       0x00010000  /* 64KB */

/* Peripheral Base Addresses */
#define S32K344_AIPS_BASE       0x40000000  /* AIPS-Lite Bridge */
#define S32K344_AIPS_SIZE       0x40000000  /* 1GB address space */

/* Core Peripherals */
#define S32K344_NVIC_BASE       0xE000E000
#define S32K344_SYSTICK_BASE    0xE000E010
#define S32K344_SCB_BASE        0xE000ED00

/* Main System Peripherals */
#define S32K344_SIUL2_BASE      0x40290000  /* System Integration Unit Lite 2 */
#define S32K344_LPUART0_BASE    0x40328000  /* Low Power UART 0 */
#define S32K344_LPUART1_BASE    0x40329000  /* Low Power UART 1 */
#define S32K344_LPUART2_BASE    0x4032A000  /* Low Power UART 2 */
#define S32K344_LPUART3_BASE    0x4032B000  /* Low Power UART 3 */
#define S32K344_FLEXCAN0_BASE   0x40304000  /* FlexCAN 0 */
#define S32K344_FLEXCAN1_BASE   0x40305000  /* FlexCAN 1 */
#define S32K344_FLEXCAN2_BASE   0x40306000  /* FlexCAN 2 */
#define S32K344_QSPI_BASE       0x40134000  /* QuadSPI */
#define S32K344_WDOG_BASE       0x40088000  /* Watchdog Timer */
#define S32K344_STM_BASE        0x4003C000  /* System Timer Module */
#define S32K344_PIT_BASE        0x40037000  /* Periodic Interrupt Timer */
#define S32K344_ADC0_BASE       0x40030000  /* ADC 0 */
#define S32K344_ADC1_BASE       0x40031000  /* ADC 1 */
#define S32K344_ADC2_BASE       0x40032000  /* ADC 2 */

/* Clock frequencies */
#define S32K344_SYSCLK_FRQ      160000000   /* 160 MHz system clock */
#define S32K344_REFCLK_FRQ      16000000    /* 16 MHz reference clock */

typedef struct S32K344MachineState {
    MachineState parent;
    
    ARMv7MState armv7m;
    
    /* Clocks */
    Clock *sysclk;
    Clock *refclk;
    
    /* Memory regions */
    MemoryRegion flash;
    MemoryRegion sram;
    MemoryRegion dtcm;
    MemoryRegion itcm;
    MemoryRegion aips;
    
    /* Peripherals */
    DeviceState *siul2;
    DeviceState *lpuart[4];
    DeviceState *flexcan[3];
    DeviceState *wdog;
    DeviceState *stm;
    DeviceState *pit;
    DeviceState *adc[3];
    DeviceState *qspi;
    
    /* Board configuration */
    bool enable_flash;
    bool enable_semihosting;
    uint32_t flash_size;
    uint32_t sram_size;
    
} S32K344MachineState;

#define TYPE_S32K344_MACHINE MACHINE_TYPE_NAME("s32k344")
OBJECT_DECLARE_SIMPLE_TYPE(S32K344MachineState, S32K344_MACHINE)

/* Helper function to create unimplemented device regions */
static void create_unimplemented_device(const char *name, hwaddr base, hwaddr size)
{
    DeviceState *dev = qdev_new(TYPE_UNIMPLEMENTED_DEVICE);
    qdev_prop_set_string(dev, "name", name);
    qdev_prop_set_uint64(dev, "size", size);
    sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, base);
}

/* Initialize memory regions */
static void s32k344_memory_init(S32K344MachineState *s)
{
    MachineState *ms = MACHINE(s);
    MemoryRegion *system_memory = get_system_memory();
    
    /* Program Flash Memory */
    memory_region_init_rom(&s->flash, OBJECT(s), "s32k344.flash", 
                          s->flash_size, &error_fatal);
    memory_region_add_subregion(system_memory, S32K344_FLASH_BASE, &s->flash);
    
    /* System RAM */
    memory_region_init_ram(&s->sram, OBJECT(s), "s32k344.sram", 
                          s->sram_size, &error_fatal);
    memory_region_add_subregion(system_memory, S32K344_SRAM_BASE, &s->sram);
    
    /* Data Tightly Coupled Memory (DTCM) */
    memory_region_init_ram(&s->dtcm, OBJECT(s), "s32k344.dtcm", 
                          S32K344_DTCM_SIZE, &error_fatal);
    memory_region_add_subregion(system_memory, S32K344_DTCM_BASE, &s->dtcm);
    
    /* Instruction Tightly Coupled Memory (ITCM) */
    memory_region_init_ram(&s->itcm, OBJECT(s), "s32k344.itcm", 
                          S32K344_ITCM_SIZE, &error_fatal);
    memory_region_add_subregion(system_memory, S32K344_ITCM_BASE, &s->itcm);
    
    /* AIPS-Lite Bridge region for peripherals */
    memory_region_init(&s->aips, OBJECT(s), "s32k344.aips", S32K344_AIPS_SIZE);
    memory_region_add_subregion(system_memory, S32K344_AIPS_BASE, &s->aips);
    
    qemu_log_mask(LOG_GUEST_ERROR, 
                  "S32K344: Memory layout initialized - Flash: 0x%08x, SRAM: 0x%08x\n",
                  s->flash_size, s->sram_size);
}

/* Initialize clocks */
static void s32k344_clock_init(S32K344MachineState *s)
{
    s->sysclk = clock_new(OBJECT(s), "SYSCLK");
    s->refclk = clock_new(OBJECT(s), "REFCLK");
    
    clock_set_hz(s->sysclk, S32K344_SYSCLK_FRQ);
    clock_set_hz(s->refclk, S32K344_REFCLK_FRQ);
    
    qemu_log_mask(LOG_GUEST_ERROR, 
                  "S32K344: Clocks initialized - SYSCLK: %d Hz, REFCLK: %d Hz\n",
                  S32K344_SYSCLK_FRQ, S32K344_REFCLK_FRQ);
}

/* Initialize peripherals */
static void s32k344_peripheral_init(S32K344MachineState *s)
{
    /* System Integration Unit Lite 2 (SIUL2) - GPIO controller */
    create_unimplemented_device("s32k344.siul2", S32K344_SIUL2_BASE, 0x1000);
    
    /* Low Power UARTs */
    for (int i = 0; i < 4; i++) {
        create_unimplemented_device(g_strdup_printf("s32k344.lpuart%d", i),
                                   S32K344_LPUART0_BASE + (i * 0x1000), 0x1000);
    }
    
    /* FlexCAN Controllers */
    for (int i = 0; i < 3; i++) {
        create_unimplemented_device(g_strdup_printf("s32k344.flexcan%d", i),
                                   S32K344_FLEXCAN0_BASE + (i * 0x1000), 0x1000);
    }
    
    /* ADC Controllers */
    for (int i = 0; i < 3; i++) {
        create_unimplemented_device(g_strdup_printf("s32k344.adc%d", i),
                                   S32K344_ADC0_BASE + (i * 0x1000), 0x1000);
    }
    
    /* Timer and Watchdog */
    create_unimplemented_device("s32k344.wdog", S32K344_WDOG_BASE, 0x1000);
    create_unimplemented_device("s32k344.stm", S32K344_STM_BASE, 0x1000);
    create_unimplemented_device("s32k344.pit", S32K344_PIT_BASE, 0x1000);
    
    /* QuadSPI */
    create_unimplemented_device("s32k344.qspi", S32K344_QSPI_BASE, 0x1000);
    
    qemu_log_mask(LOG_GUEST_ERROR, 
                  "S32K344: Peripherals initialized (placeholder implementations)\n");
}

/* Initialize the ARMv7M core */
static void s32k344_core_init(S32K344MachineState *s)
{
    MachineState *ms = MACHINE(s);
    DeviceState *armv7m;
    
    object_initialize_child(OBJECT(s), "armv7m", &s->armv7m, TYPE_ARMV7M);
    armv7m = DEVICE(&s->armv7m);
    
    /* Configure the Cortex-M7 core */
    qdev_prop_set_uint32(armv7m, "num-irq", 256);  /* S32K344 has 256 IRQs */
    qdev_prop_set_string(armv7m, "cpu-type", ARM_CPU_TYPE_NAME("cortex-m7"));
    
    /* Connect system clock */
    qdev_connect_clock_in(armv7m, "cpuclk", s->sysclk);
    qdev_connect_clock_in(armv7m, "refclk", s->refclk);
    
    /* Enable floating point unit */
    qdev_prop_set_bit(armv7m, "enable-bitband", true);
    qdev_prop_set_bit(armv7m, "start-powered-off", false);
    
    /* Set memory map */
    object_property_set_link(OBJECT(&s->armv7m), "memory", 
                            OBJECT(get_system_memory()), &error_abort);
    
    sysbus_realize(SYS_BUS_DEVICE(&s->armv7m), &error_fatal);
    
    qemu_log_mask(LOG_GUEST_ERROR, 
                  "S32K344: ARM Cortex-M7 core initialized at %d MHz\n",
                  S32K344_SYSCLK_FRQ / 1000000);
}

/* Main machine initialization */
static void s32k344_machine_init(MachineState *ms)
{
    S32K344MachineState *s = S32K344_MACHINE(ms);
    
    /* Validate machine configuration */
    if (ms->ram_size > S32K344_SRAM_SIZE) {
        error_report("S32K344: RAM size (%u) exceeds maximum (%u)",
                    (unsigned)ms->ram_size, S32K344_SRAM_SIZE);
        exit(1);
    }
    
    /* Set default configuration */
    s->flash_size = S32K344_FLASH_SIZE;
    s->sram_size = ms->ram_size ?: S32K344_SRAM_SIZE;
    s->enable_flash = true;
    s->enable_semihosting = false;
    
    /* Initialize subsystems */
    s32k344_clock_init(s);
    s32k344_memory_init(s);
    s32k344_core_init(s);
    s32k344_peripheral_init(s);
    
    /* Load firmware if specified */
    if (ms->firmware) {
        arm_load_kernel(&s->armv7m.cpu, ms, S32K344_FLASH_BASE, 
                       s->flash_size, 0, NULL);
    }
    
    qemu_log_mask(LOG_GUEST_ERROR, 
                  "S32K344: Machine initialization complete\n");
}

/* Machine class initialization */
static void s32k344_machine_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);
    
    mc->desc = "NXP S32K344 Automotive Microcontroller (ARM Cortex-M7)";
    mc->init = s32k344_machine_init;
    mc->max_cpus = 1;
    mc->min_cpus = 1;
    mc->default_cpus = 1;
    mc->default_ram_size = S32K344_SRAM_SIZE;
    mc->default_ram_id = "s32k344.sram";
    mc->ignore_memory_transaction_failures = true;
    mc->default_cpu_type = ARM_CPU_TYPE_NAME("cortex-m7");
    
    /* Board specific properties */
    mc->no_floppy = 1;
    mc->no_cdrom = 1;
    mc->no_parallel = 1;
    mc->no_serial = 1;
    mc->no_soundhw = 1;
    mc->no_sdcard = 1;
    mc->is_default = false;
}

/* Object initialization */
static void s32k344_machine_init_obj(Object *obj)
{
    S32K344MachineState *s = S32K344_MACHINE(obj);
    
    /* Initialize any additional object properties here */
    (void)s;  /* Suppress unused variable warning */
}

static const TypeInfo s32k344_machine_type = {
    .name = TYPE_S32K344_MACHINE,
    .parent = TYPE_MACHINE,
    .instance_size = sizeof(S32K344MachineState),
    .instance_init = s32k344_machine_init_obj,
    .class_init = s32k344_machine_class_init,
};

static void s32k344_machine_register_types(void)
{
    type_register_static(&s32k344_machine_type);
}

type_init(s32k344_machine_register_types) 