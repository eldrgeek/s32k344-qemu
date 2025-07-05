/*
 * NXP S32K344 SIUL2 (System Integration Unit Lite 2) - GPIO Controller
 *
 * The SIUL2 provides GPIO functionality, pin multiplexing, and external interrupt
 * handling for the S32K344 microcontroller.
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
#include "qemu/log.h"
#include "qemu/module.h"
#include "hw/qdev-properties.h"
#include "hw/irq.h"
#include "hw/sysbus.h"
#include "migration/vmstate.h"
#include "hw/misc/s32k344_siul2.h"

/* SIUL2 Register Offsets */
#define SIUL2_MIDR1         0x0004    /* MCU ID Register 1 */
#define SIUL2_MIDR2         0x0008    /* MCU ID Register 2 */
#define SIUL2_DISR0         0x0010    /* DMA/Interrupt Status Register 0 */
#define SIUL2_DIRER0        0x0018    /* DMA/Interrupt Request Enable Register 0 */
#define SIUL2_DIRSR0        0x001C    /* DMA/Interrupt Request Select Register 0 */
#define SIUL2_IREER0        0x0020    /* Interrupt Rising-Edge Event Enable Register 0 */
#define SIUL2_IFEER0        0x0024    /* Interrupt Falling-Edge Event Enable Register 0 */
#define SIUL2_IFER0         0x0028    /* Interrupt Filter Enable Register 0 */

/* Pin Control Registers (MSCR) - 0x0240 to 0x04FC */
#define SIUL2_MSCR_BASE     0x0240
#define SIUL2_MSCR(n)       (SIUL2_MSCR_BASE + ((n) * 4))

/* GPIO Pad Data Output Registers (GPDO) - 0x1300 to 0x1480 */
#define SIUL2_GPDO_BASE     0x1300
#define SIUL2_GPDO(n)       (SIUL2_GPDO_BASE + (n))

/* GPIO Pad Data Input Registers (GPDI) - 0x1500 to 0x1680 */
#define SIUL2_GPDI_BASE     0x1500
#define SIUL2_GPDI(n)       (SIUL2_GPDI_BASE + (n))

/* MSCR Register Fields */
#define SIUL2_MSCR_SSS_MASK     0x7        /* Source Signal Select */
#define SIUL2_MSCR_SRE          (1 << 14)  /* Slew Rate Control */
#define SIUL2_MSCR_PUE          (1 << 13)  /* Pull-up Enable */
#define SIUL2_MSCR_PUS          (1 << 12)  /* Pull-up/down Select */
#define SIUL2_MSCR_INV          (1 << 11)  /* Inverted Value */
#define SIUL2_MSCR_IBE          (1 << 19)  /* Input Buffer Enable */
#define SIUL2_MSCR_OBE          (1 << 21)  /* Output Buffer Enable */
#define SIUL2_MSCR_ODE          (1 << 20)  /* Open Drain Enable */

static uint64_t s32k344_siul2_read(void *opaque, hwaddr addr, unsigned size)
{
    S32K344SIUL2State *s = S32K344_SIUL2(opaque);
    uint32_t value = 0;

    switch (addr) {
    case SIUL2_MIDR1:
        value = s->midr1;
        break;
    case SIUL2_MIDR2:
        value = s->midr2;
        break;
    case SIUL2_DISR0:
        value = s->disr0;
        break;
    case SIUL2_DIRER0:
        value = s->direr0;
        break;
    case SIUL2_DIRSR0:
        value = s->dirsr0;
        break;
    case SIUL2_IREER0:
        value = s->ireer0;
        break;
    case SIUL2_IFEER0:
        value = s->ifeer0;
        break;
    case SIUL2_IFER0:
        value = s->ifer0;
        break;
    default:
        if (addr >= SIUL2_MSCR_BASE && addr < SIUL2_MSCR_BASE + (S32K344_SIUL2_PIN_COUNT * 4)) {
            int pin = (addr - SIUL2_MSCR_BASE) / 4;
            if (pin < S32K344_SIUL2_PIN_COUNT) {
                value = s->mscr[pin];
            }
        } else if (addr >= SIUL2_GPDO_BASE && addr < SIUL2_GPDO_BASE + S32K344_SIUL2_PIN_COUNT) {
            int pin = addr - SIUL2_GPDO_BASE;
            if (pin < S32K344_SIUL2_PIN_COUNT) {
                value = s->gpdo[pin];
            }
        } else if (addr >= SIUL2_GPDI_BASE && addr < SIUL2_GPDI_BASE + S32K344_SIUL2_PIN_COUNT) {
            int pin = addr - SIUL2_GPDI_BASE;
            if (pin < S32K344_SIUL2_PIN_COUNT) {
                value = s->gpdi[pin];
            }
        } else {
            qemu_log_mask(LOG_GUEST_ERROR,
                         "%s: Bad read offset 0x%" HWADDR_PRIx "\n",
                         __func__, addr);
        }
        break;
    }

    qemu_log_mask(LOG_TRACE, "%s: addr=0x%" HWADDR_PRIx " value=0x%08x\n",
                  __func__, addr, value);
    return value;
}

static void s32k344_siul2_write(void *opaque, hwaddr addr, uint64_t value, unsigned size)
{
    S32K344SIUL2State *s = S32K344_SIUL2(opaque);

    qemu_log_mask(LOG_TRACE, "%s: addr=0x%" HWADDR_PRIx " value=0x%08" PRIx64 "\n",
                  __func__, addr, value);

    switch (addr) {
    case SIUL2_DISR0:
        /* Write 1 to clear interrupt status bits */
        s->disr0 &= ~value;
        break;
    case SIUL2_DIRER0:
        s->direr0 = value;
        break;
    case SIUL2_DIRSR0:
        s->dirsr0 = value;
        break;
    case SIUL2_IREER0:
        s->ireer0 = value;
        break;
    case SIUL2_IFEER0:
        s->ifeer0 = value;
        break;
    case SIUL2_IFER0:
        s->ifer0 = value;
        break;
    default:
        if (addr >= SIUL2_MSCR_BASE && addr < SIUL2_MSCR_BASE + (S32K344_SIUL2_PIN_COUNT * 4)) {
            int pin = (addr - SIUL2_MSCR_BASE) / 4;
            if (pin < S32K344_SIUL2_PIN_COUNT) {
                s->mscr[pin] = value;
                /* Update GPIO direction and configuration */
                s->gpio_direction[pin] = (value & SIUL2_MSCR_OBE) ? 1 : 0;
                s->gpio_input_enabled[pin] = (value & SIUL2_MSCR_IBE) ? 1 : 0;
            }
        } else if (addr >= SIUL2_GPDO_BASE && addr < SIUL2_GPDO_BASE + S32K344_SIUL2_PIN_COUNT) {
            int pin = addr - SIUL2_GPDO_BASE;
            if (pin < S32K344_SIUL2_PIN_COUNT) {
                s->gpdo[pin] = value & 1;
                /* Update external GPIO line if configured as output */
                if (s->gpio_direction[pin]) {
                    qemu_set_irq(s->gpio_out[pin], s->gpdo[pin]);
                }
            }
        } else if (addr >= SIUL2_GPDI_BASE && addr < SIUL2_GPDI_BASE + S32K344_SIUL2_PIN_COUNT) {
            /* GPDI is read-only, log warning */
            qemu_log_mask(LOG_GUEST_ERROR,
                         "%s: Write to read-only GPDI register 0x%" HWADDR_PRIx "\n",
                         __func__, addr);
        } else {
            qemu_log_mask(LOG_GUEST_ERROR,
                         "%s: Bad write offset 0x%" HWADDR_PRIx "\n",
                         __func__, addr);
        }
        break;
    }
}

static const MemoryRegionOps s32k344_siul2_ops = {
    .read = s32k344_siul2_read,
    .write = s32k344_siul2_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 1,
        .max_access_size = 4,
    },
};

static void s32k344_siul2_gpio_set(void *opaque, int pin, int level)
{
    S32K344SIUL2State *s = S32K344_SIUL2(opaque);

    if (pin >= S32K344_SIUL2_PIN_COUNT) {
        return;
    }

    /* Update input data register if input is enabled */
    if (s->gpio_input_enabled[pin]) {
        s->gpdi[pin] = level & 1;
        
        /* Handle interrupt generation if enabled */
        if (s->direr0 & (1 << pin)) {
            bool old_level = s->gpio_prev_level[pin];
            bool trigger = false;
            
            if ((s->ireer0 & (1 << pin)) && !old_level && level) {
                /* Rising edge */
                trigger = true;
            }
            if ((s->ifeer0 & (1 << pin)) && old_level && !level) {
                /* Falling edge */
                trigger = true;
            }
            
            if (trigger) {
                s->disr0 |= (1 << pin);
                /* Generate interrupt if not masked */
                qemu_irq_raise(s->irq);
            }
        }
        
        s->gpio_prev_level[pin] = level;
    }
}

static void s32k344_siul2_realize(DeviceState *dev, Error **errp)
{
    S32K344SIUL2State *s = S32K344_SIUL2(dev);
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);

    memory_region_init_io(&s->iomem, OBJECT(s), &s32k344_siul2_ops, s,
                          TYPE_S32K344_SIUL2, 0x4000);
    sysbus_init_mmio(sbd, &s->iomem);

    /* Initialize IRQ line */
    sysbus_init_irq(sbd, &s->irq);

    /* Initialize GPIO lines */
    qdev_init_gpio_in(dev, s32k344_siul2_gpio_set, S32K344_SIUL2_PIN_COUNT);
    qdev_init_gpio_out(dev, s->gpio_out, S32K344_SIUL2_PIN_COUNT);
}

static void s32k344_siul2_reset(DeviceState *dev)
{
    S32K344SIUL2State *s = S32K344_SIUL2(dev);

    /* Reset to default values */
    s->midr1 = 0x57373001;  /* S32K344 MCU ID */
    s->midr2 = 0x00000000;
    s->disr0 = 0x00000000;
    s->direr0 = 0x00000000;
    s->dirsr0 = 0x00000000;
    s->ireer0 = 0x00000000;
    s->ifeer0 = 0x00000000;
    s->ifer0 = 0x00000000;

    /* Reset GPIO configuration */
    memset(s->mscr, 0, sizeof(s->mscr));
    memset(s->gpdo, 0, sizeof(s->gpdo));
    memset(s->gpdi, 0, sizeof(s->gpdi));
    memset(s->gpio_direction, 0, sizeof(s->gpio_direction));
    memset(s->gpio_input_enabled, 0, sizeof(s->gpio_input_enabled));
    memset(s->gpio_prev_level, 0, sizeof(s->gpio_prev_level));
}

static const VMStateDescription vmstate_s32k344_siul2 = {
    .name = TYPE_S32K344_SIUL2,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32(midr1, S32K344SIUL2State),
        VMSTATE_UINT32(midr2, S32K344SIUL2State),
        VMSTATE_UINT32(disr0, S32K344SIUL2State),
        VMSTATE_UINT32(direr0, S32K344SIUL2State),
        VMSTATE_UINT32(dirsr0, S32K344SIUL2State),
        VMSTATE_UINT32(ireer0, S32K344SIUL2State),
        VMSTATE_UINT32(ifeer0, S32K344SIUL2State),
        VMSTATE_UINT32(ifer0, S32K344SIUL2State),
        VMSTATE_UINT32_ARRAY(mscr, S32K344SIUL2State, S32K344_SIUL2_PIN_COUNT),
        VMSTATE_UINT8_ARRAY(gpdo, S32K344SIUL2State, S32K344_SIUL2_PIN_COUNT),
        VMSTATE_UINT8_ARRAY(gpdi, S32K344SIUL2State, S32K344_SIUL2_PIN_COUNT),
        VMSTATE_END_OF_LIST()
    }
};

static void s32k344_siul2_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = s32k344_siul2_realize;
    dc->reset = s32k344_siul2_reset;
    dc->vmsd = &vmstate_s32k344_siul2;
    dc->desc = "S32K344 SIUL2 GPIO Controller";
}

static const TypeInfo s32k344_siul2_info = {
    .name = TYPE_S32K344_SIUL2,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(S32K344SIUL2State),
    .class_init = s32k344_siul2_class_init,
};

static void s32k344_siul2_register_types(void)
{
    type_register_static(&s32k344_siul2_info);
}

type_init(s32k344_siul2_register_types) 