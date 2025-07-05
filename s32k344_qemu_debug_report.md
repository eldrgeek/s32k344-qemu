# S32K344 QEMU Integration Debug Report: "Object is not an instance of type device" Error

## Problem Description

During the integration of the NXP S32K344 microcontroller into QEMU, a persistent runtime error "Object is not an instance of type device" is encountered. This error originates from `qemu-upstream/include/hw/qdev-core.h` at line 77, specifically within the `OBJECT_CHECK` macro, which is used by `DEVICE()` macro.

The error occurs when attempting to connect the LPUART (Low Power UART) interrupt to the ARMv7M CPU's GPIO input in the `s32k344_peripheral_init` function within `qemu-upstream/hw/arm/s32k344/s32k344_machine.c`. The specific line causing the error is:

```c
qdev_connect_gpio_out(s->lpuart[i], 0, 
                     qdev_get_gpio_in(armv7m_dev, 48 + i));
```

Despite ensuring that `s->armv7m` (which is passed as `armv7m_dev` to `s32k344_peripheral_init`) is properly initialized and realized as a `TYPE_ARMV7M` device via `sysbus_realize(SYS_BUS_DEVICE(&s->armv7m), &error_fatal);` in `s32k344_core_init`, the `qdev_get_gpio_in` function appears to receive an object that QEMU's QOM (QEMU Object Model) system does not recognize as a valid `DeviceState` instance. This suggests a potential issue with the QOM hierarchy, object lifecycle, or type registration related to how the ARMv7M device is being handled or referenced.

Several debugging attempts have been made, including:
*   Passing `DEVICE(&s->armv7m.cpu.qdev)` and `DEVICE(s->armv7m.cpu)` to `qdev_get_gpio_in`, both leading to compilation errors or the same runtime error.
*   Reverting the LPUART IRQ connection back to `DEVICE(&s->armv7m)`.
*   Reordering initialization calls in `s32k344_machine.c` to ensure `s32k344_core_init` (which initializes `ARMv7MState`) is called *before* `s32k344_peripheral_init`.
*   Attempting to use GDB within the Docker container to get a backtrace, which has been unsuccessful due to symbol resolution issues and GDB connection problems.

The core problem remains that the `DEVICE` macro, which performs a runtime type check, fails for the `armv7m_dev` argument when `qdev_get_gpio_in` is called.

## Relevant Files

### `qemu-upstream/hw/arm/s32k344/s32k344_machine.c`

```c
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
#include "hw/misc/unimp.h"
#include "exec/address-spaces.h"
#include "sysemu/sysemu.h"
#include "qemu/log.h"
#include "qemu/error-report.h"
#include "s32k344.h"
#include "s32k344_lpuart.h"

/* Peripheral Base Addresses */
#define S32K344_AIPS_BASE       0x40000000  /* AIPS-Lite Bridge */
#define S32K344_AIPS_SIZE       0x40000000  /* 1GB address space */

/* Core Peripherals */
#define S32K344_SCB_BASE        0xE000ED00

/* Additional peripherals not in header */
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

/* Use the S32K344MachineState from the header file */

/* Helper function for creating unimplemented devices is in unimp.h */

/* Initialize memory regions */
static void s32k344_memory_init(S32K344MachineState *s)
{
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
static void s32k344_peripheral_init(S32K344MachineState *s, DeviceState *armv7m_dev)
{
    /* System Integration Unit Lite 2 (SIUL2) - GPIO controller */
    create_unimplemented_device("s32k344.siul2", S32K344_SIUL2_BASE, 0x1000);
    
    /* Low Power UARTs */
    for (int i = 0; i < 4; i++) {
        s->lpuart[i] = qdev_new(TYPE_S32K344_LPUART);
        
        /* Connect to serial console for LPUART0 */
        if (i == 0) {
            qdev_prop_set_chr(s->lpuart[i], "chardev", serial_hd(0));
        }
        
        sysbus_realize_and_unref(SYS_BUS_DEVICE(s->lpuart[i]), &error_fatal);
        sysbus_mmio_map(SYS_BUS_DEVICE(s->lpuart[i]), 0, 
                       S32K344_LPUART0_BASE + (i * 0x1000));
        
        /* Connect LPUART IRQ to NVIC */
        qdev_connect_gpio_out(s->lpuart[i], 0, 
                             qdev_get_gpio_in(armv7m_dev, 48 + i));
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
                  "S32K344: Peripherals initialized - LPUART0-3 functional, others placeholder\n");
}

/* Initialize the ARMv7M core */
static void s32k344_core_init(S32K344MachineState *s)
{
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
    s32k344_core_init(s);
    s32k344_clock_init(s);
    s32k344_memory_init(s);
    s32k344_peripheral_init(s, DEVICE(&s->armv7m));
    
    /* Load firmware if specified */
    if (ms->firmware) {
        /* For ARM Cortex-M, firmware loading is handled by the ARMv7M container */
        qemu_log_mask(LOG_GUEST_ERROR, 
                      "S32K344: Firmware loading via kernel option not supported for Cortex-M\n");
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

static void s32k344_machine_register_types(void)
{
    type_register_instance_as_parent_and_class(sizeof(S32K344MachineState), 
                                               TYPE_S32K344_MACHINE, 
                                               TYPE_MACHINE, 
                                               s32k344_machine_init_obj, 
                                               s32k344_machine_class_init,
                                               NULL);
}

type_init(s32k344_machine_register_types)

```

### `qemu-upstream/include/hw/arm/armv7m.h`

```c
/*
 * ARMv7M CPU object
 *
 * Copyright (c) 2017 Linaro Ltd
 * Written by Peter Maydell <peter.maydell@linaro.org>
 *
 * This code is licensed under the GPL version 2 or later.
 */

#ifndef HW_ARM_ARMV7M_H
#define HW_ARM_ARMV7M_H

#include "hw/sysbus.h"
#include "hw/intc/armv7m_nvic.h"
#include "hw/misc/armv7m_ras.h"
#include "target/arm/idau.h"
#include "qom/object.h"
#include "hw/clock.h"

#define TYPE_BITBAND "ARM-bitband-memory"
OBJECT_DECLARE_SIMPLE_TYPE(BitBandState, BITBAND)

struct BitBandState {
    /*< private >*/
    SysBusDevice parent_obj;
    /*< public >*/

    AddressSpace source_as;
    MemoryRegion iomem;
    uint32_t base;
    MemoryRegion *source_memory;
};

#define TYPE_ARMV7M "armv7m"
OBJECT_DECLARE_SIMPLE_TYPE(ARMv7MState, ARMV7M)

#define ARMV7M_NUM_BITBANDS 2

/* ARMv7M container object.
 * + Unnamed GPIO input lines: external IRQ lines for the NVIC
 * + Named GPIO output SYSRESETREQ: signalled for guest AIRCR.SYSRESETREQ.
 *   If this GPIO is not wired up then the NVIC will default to performing
 *   a qemu_system_reset_request(SHUTDOWN_CAUSE_GUEST_RESET).
 * + Property "cpu-type": CPU type to instantiate
 * + Property "num-irq": number of external IRQ lines
 * + Property "num-prio-bits": number of priority bits in the NVIC
 * + Property "memory": MemoryRegion defining the physical address space
 *   that CPU accesses see. (The NVIC, bitbanding and other CPU-internal
 *   devices will be automatically layered on top of this view.)
 * + Property "idau": IDAU interface (forwarded to CPU object)
 * + Property "init-svtor": secure VTOR reset value (forwarded to CPU object)
 * + Property "init-nsvtor": non-secure VTOR reset value (forwarded to CPU object)
 * + Property "vfp": enable VFP (forwarded to CPU object)
 * + Property "dsp": enable DSP (forwarded to CPU object)
 * + Property "enable-bitband": expose bitbanded IO
 * + Property "mpu-ns-regions": number of Non-Secure MPU regions (forwarded
 *   to CPU object pmsav7-dregion property; default is whatever the default
 *   for the CPU is)
 * + Property "mpu-s-regions": number of Secure MPU regions (default is
 *   whatever the default for the CPU is; must currently be set to the same
 *   value as mpu-ns-regions if the CPU implements the Security Extension)
 * + Clock input "refclk" is the external reference clock for the systick timers
 * + Clock input "cpuclk" is the main CPU clock
 */
struct ARMv7MState {
    /*< private >*/
    SysBusDevice parent_obj;
    /*< public >*/
    NVICState nvic;
    BitBandState bitband[ARMV7M_NUM_BITBANDS];
    ARMCPU *cpu;
    ARMv7MRAS ras;
    SysTickState systick[M_REG_NUM_BANKS];

    /* MemoryRegion we pass to the CPU, with our devices layered on
     * top of the ones the board provides in board_memory.
     */
    MemoryRegion container;
    /*
     * MemoryRegion which passes the transaction to either the S or the
     * NS systick device depending on the transaction attributes
     */
    MemoryRegion systickmem;
    /*
     * MemoryRegion which enforces the S/NS handling of the systick
     * device NS alias region and passes the transaction to the
     * NS systick device if appropriate.
     */
    MemoryRegion systick_ns_mem;
    /* Ditto, for the sysregs region provided by the NVIC */
    MemoryRegion sysreg_ns_mem;
    /* MR providing default PPB behaviour */
    MemoryRegion defaultmem;

    Clock *refclk;
    Clock *cpuclk;

    /* Properties */
    char *cpu_type;
    /* MemoryRegion the board provides to us (with its devices, RAM, etc) */
    MemoryRegion *board_memory;
    Object *idau;
    uint32_t init_svtor;
    uint32_t init_nsvtor;
    uint32_t mpu_ns_regions;
    uint32_t mpu_s_regions;
    bool enable_bitband;
    bool start_powered_off;
    bool vfp;
    bool dsp;
};

#endif
```

### `qemu-upstream/include/hw/qdev-core.h` (Relevant excerpt)

```c
// ... existing code ...
#define TYPE_DEVICE "device"
OBJECT_DECLARE_TYPE(DeviceState, DeviceClass, DEVICE)

typedef enum DeviceCategory {
    DEVICE_CATEGORY_BRIDGE,
    DEVICE_CATEGORY_USB,
    // ... other categories ...
} DeviceCategory;

typedef void (*DeviceRealize)(DeviceState *dev, Error **errp);
typedef void (*DeviceUnrealize)(DeviceState *dev);
typedef void (*DeviceReset)(DeviceState *dev);

// ... existing code ...
struct DeviceState {
    /* private: */
    Object parent_obj;
    /* public: */

    /**
     * @id: global device id
     */
    char *id;
    /**
     * @canonical_path: canonical path of realized device in the QOM tree
     */
    char *canonical_path;
    /**
     * @realized: has device been realized?
     */
    bool realized;
    // ... existing code ...
};
// ... existing code ...
```

### `qemu-upstream/hw/arm/s32k344/s32k344_lpuart.c`

```c
/*
 * NXP S32K344 LPUART (Low Power Universal Asynchronous Receiver/Transmitter)
 *
 * Copyright (c) 2024 
 * Written by QEMU S32K344 Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "qemu/osdep.h"
#include "hw/irq.h"
#include "hw/sysbus.h"
#include "hw/resettable.h"
#include "hw/qdev-properties.h"
#include "hw/qdev-properties-system.h"
#include "chardev/char-fe.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "qapi/error.h"
#include "s32k344.h"
#include "s32k344_lpuart.h"

/* Remove duplicate definitions - these are now in s32k344_lpuart.h */

static void s32k344_lpuart_update_irq(S32K344LPUARTState *s)
{
    bool irq_pending = false;
    
    /* Check transmit interrupt */
    if ((s->regs[LPUART_CTRL / 4] & LPUART_CTRL_TIE) &&
        (s->regs[LPUART_STAT / 4] & LPUART_STAT_TDRE)) {
        irq_pending = true;
    }
    
    /* Check receive interrupt */
    if ((s->regs[LPUART_CTRL / 4] & LPUART_CTRL_RIE) &&
        (s->regs[LPUART_STAT / 4] & LPUART_STAT_RDRF)) {
        irq_pending = true;
    }
    
    qemu_set_irq(s->irq, irq_pending);
}

static uint64_t s32k344_lpuart_read(void *opaque, hwaddr addr, unsigned size)
{
    S32K344LPUARTState *s = S32K344_LPUART(opaque);
    uint32_t reg_idx = addr / 4;
    uint32_t value = 0;
    
    if (reg_idx >= ARRAY_SIZE(s->regs)) {
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Invalid register offset 0x%x\n",
                      __func__, (unsigned)addr);
        return 0;
    }
    
    switch (addr) {
    case LPUART_VERID:
        value = 0x04030003;  /* Version 4.3.3 */
        break;
    case LPUART_PARAM:
        value = 0x0F0F;      /* 16-byte TX/RX FIFO */
        break;
    case LPUART_DATA:
        if (s->rx_fifo_len > 0) {
            value = s->rx_fifo[s->rx_fifo_head];
            s->rx_fifo_head = (s->rx_fifo_head + 1) % 16;
            s->rx_fifo_len--;
            if (s->rx_fifo_len == 0) {
                s->regs[LPUART_STAT / 4] &= ~LPUART_STAT_RDRF;
            }
        }
        break;
    case LPUART_STAT:
        /* Update transmit status */
        s->regs[LPUART_STAT / 4] |= LPUART_STAT_TDRE | LPUART_STAT_TC;
        if (s->rx_fifo_len > 0) {
            s->regs[LPUART_STAT / 4] |= LPUART_STAT_RDRF;
        }
        value = s->regs[reg_idx];
        break;
    default:
        value = s->regs[reg_idx];
        break;
    }
    
    return value;
}

static void s32k344_lpuart_write(void *opaque, hwaddr addr, uint64_t value,
                                 unsigned size)
{
    S32K344LPUARTState *s = S32K344_LPUART(opaque);
    uint32_t reg_idx = addr / 4;
    
    if (reg_idx >= ARRAY_SIZE(s->regs)) {
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Invalid register offset 0x%x\n",
                      __func__, (unsigned)addr);
        return;
    }
    
    switch (addr) {
    case LPUART_DATA:
        if (s->regs[LPUART_CTRL / 4] & LPUART_CTRL_TE) {
            /* Transmit the character */
            uint8_t ch = value & 0xFF;
            qemu_chr_fe_write_all(&s->chr, &ch, 1);
        }
        break;
    case LPUART_STAT:
        /* Write-1-to-clear status bits */
        s->regs[reg_idx] &= ~(value & (LPUART_STAT_IDLE | LPUART_STAT_OR |
                                       LPUART_STAT_NF | LPUART_STAT_FE |
                                       LPUART_STAT_PF));
        break;
    case LPUART_CTRL:
        s->regs[reg_idx] = value;
        /* Update interrupt state when interrupt enables change */
        s32k344_lpuart_update_irq(s);
        break;
    default:
        s->regs[reg_idx] = value;
        break;
    }
}

static const MemoryRegionOps s32k344_lpuart_ops = {
    .read = s32k344_lpuart_read,
    .write = s32k344_lpuart_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static void s32k344_lpuart_receive(void *opaque, const uint8_t *buf, int size)
{
    S32K344LPUARTState *s = S32K344_LPUART(opaque);
    
    if (!(s->regs[LPUART_CTRL / 4] & LPUART_CTRL_RE)) {
        return; /* Receiver disabled */
    }
    
    for (int i = 0; i < size && s->rx_fifo_len < 16; i++) {
        uint8_t rx_tail = (s->rx_fifo_head + s->rx_fifo_len) % 16;
        s->rx_fifo[rx_tail] = buf[i];
        s->rx_fifo_len++;
    }
    
    if (s->rx_fifo_len > 0) {
        s->regs[LPUART_STAT / 4] |= LPUART_STAT_RDRF;
        s32k344_lpuart_update_irq(s);
    }
}

static int s32k344_lpuart_can_receive(void *opaque)
{
    S32K344LPUARTState *s = S32K344_LPUART(opaque);
    return 16 - s->rx_fifo_len;
}

static void s32k344_lpuart_reset(Object *obj, ResetType type)
{
    S32K344LPUARTState *s = S32K344_LPUART(obj);
    
    memset(s->regs, 0, sizeof(s->regs));
    s->tx_fifo_len = 0;
    s->rx_fifo_len = 0;
    s->tx_fifo_head = 0;
    s->rx_fifo_head = 0;
    
    /* Set default register values */
    s->regs[LPUART_STAT / 4] = LPUART_STAT_TDRE | LPUART_STAT_TC;
    s->regs[LPUART_FIFO / 4] = LPUART_FIFO_TXFE | LPUART_FIFO_RXFE;
}

static void s32k344_lpuart_realize(DeviceState *dev, Error **errp)
{
    S32K344LPUARTState *s = S32K344_LPUART(dev);
    
    qemu_chr_fe_set_handlers(&s->chr, s32k344_lpuart_can_receive,
                             s32k344_lpuart_receive, NULL, NULL, s, NULL, true);
}

static void s32k344_lpuart_init(Object *obj)
{
    S32K344LPUARTState *s = S32K344_LPUART(obj);
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);
    
    memory_region_init_io(&s->iomem, obj, &s32k344_lpuart_ops, s,
                          TYPE_S32K344_LPUART, 0x100);
    sysbus_init_mmio(sbd, &s->iomem);
    sysbus_init_irq(sbd, &s->irq);
}

static Property s32k344_lpuart_properties[] = {
    DEFINE_PROP_CHR("chardev", S32K344LPUARTState, chr),
    DEFINE_PROP_END_OF_LIST(),
};

static void s32k344_lpuart_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    ResettableClass *rc = RESETTABLE_CLASS(klass);
    
    dc->realize = s32k344_lpuart_realize;
    dc->props = s32k344_lpuart_properties;
    rc->phases.enter = s32k344_lpuart_reset;
}

static void s32k344_lpuart_register_types(void)
{
    type_register_instance_as_parent_and_class(sizeof(S32K344LPUARTState),
                                               TYPE_S32K344_LPUART,
                                               TYPE_SYS_BUS_DEVICE,
                                               s32k344_lpuart_init,
                                               s32k344_lpuart_class_init,
                                               NULL);
}

type_init(s32k344_lpuart_register_types)