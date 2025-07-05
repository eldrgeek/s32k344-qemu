/*
 * NXP S32K344 SIUL2 GPIO Controller Header
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

#ifndef HW_MISC_S32K344_SIUL2_H
#define HW_MISC_S32K344_SIUL2_H

#include "hw/sysbus.h"
#include "qom/object.h"

#define TYPE_S32K344_SIUL2 "s32k344.siul2"
OBJECT_DECLARE_SIMPLE_TYPE(S32K344SIUL2State, S32K344_SIUL2)

/* S32K344 has up to 144 GPIO pins */
#define S32K344_SIUL2_PIN_COUNT 144

typedef struct S32K344SIUL2State {
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    qemu_irq irq;
    qemu_irq gpio_out[S32K344_SIUL2_PIN_COUNT];

    /* SIUL2 Registers */
    uint32_t midr1;         /* MCU ID Register 1 */
    uint32_t midr2;         /* MCU ID Register 2 */
    uint32_t disr0;         /* DMA/Interrupt Status Register 0 */
    uint32_t direr0;        /* DMA/Interrupt Request Enable Register 0 */
    uint32_t dirsr0;        /* DMA/Interrupt Request Select Register 0 */
    uint32_t ireer0;        /* Interrupt Rising-Edge Event Enable Register 0 */
    uint32_t ifeer0;        /* Interrupt Falling-Edge Event Enable Register 0 */
    uint32_t ifer0;         /* Interrupt Filter Enable Register 0 */

    /* Pin configuration arrays */
    uint32_t mscr[S32K344_SIUL2_PIN_COUNT];     /* Multiplexed Signal Configuration Registers */
    uint8_t gpdo[S32K344_SIUL2_PIN_COUNT];      /* GPIO Pad Data Output */
    uint8_t gpdi[S32K344_SIUL2_PIN_COUNT];      /* GPIO Pad Data Input */

    /* GPIO state tracking */
    uint8_t gpio_direction[S32K344_SIUL2_PIN_COUNT];     /* 0=input, 1=output */
    uint8_t gpio_input_enabled[S32K344_SIUL2_PIN_COUNT]; /* Input buffer enable */
    uint8_t gpio_prev_level[S32K344_SIUL2_PIN_COUNT];    /* Previous level for edge detection */
} S32K344SIUL2State;

#endif /* HW_MISC_S32K344_SIUL2_H */ 