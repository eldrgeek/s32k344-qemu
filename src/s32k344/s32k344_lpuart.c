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
#include "hw/char/char.h"
#include "hw/irq.h"
#include "hw/sysbus.h"
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

static void s32k344_lpuart_reset(DeviceState *dev)
{
    S32K344LPUARTState *s = S32K344_LPUART(dev);
    
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
    
    dc->reset = s32k344_lpuart_reset;
    dc->realize = s32k344_lpuart_realize;
    device_class_set_props(dc, s32k344_lpuart_properties);
}

static const TypeInfo s32k344_lpuart_info = {
    .name = TYPE_S32K344_LPUART,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(S32K344LPUARTState),
    .instance_init = s32k344_lpuart_init,
    .class_init = s32k344_lpuart_class_init,
};

static void s32k344_lpuart_register_types(void)
{
    type_register_static(&s32k344_lpuart_info);
}

type_init(s32k344_lpuart_register_types) 