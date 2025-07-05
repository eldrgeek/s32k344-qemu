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

#ifndef S32K344_LPUART_H
#define S32K344_LPUART_H

#include "hw/sysbus.h"
#include "chardev/char-fe.h"

#define TYPE_S32K344_LPUART "s32k344-lpuart"
OBJECT_DECLARE_SIMPLE_TYPE(S32K344LPUARTState, S32K344_LPUART)

/* LPUART Register offsets */
#define LPUART_VERID    0x00    /* Version ID Register */
#define LPUART_PARAM    0x04    /* Parameter Register */
#define LPUART_GLOBAL   0x08    /* Global Register */
#define LPUART_PINCFG   0x0C    /* Pin Configuration Register */
#define LPUART_BAUD     0x10    /* Baud Rate Register */
#define LPUART_STAT     0x14    /* Status Register */
#define LPUART_CTRL     0x18    /* Control Register */
#define LPUART_DATA     0x1C    /* Data Register */
#define LPUART_MATCH    0x20    /* Match Address Register */
#define LPUART_MODIR    0x24    /* Modem IrDA Register */
#define LPUART_FIFO     0x28    /* FIFO Register */
#define LPUART_WATER    0x2C    /* Watermark Register */

/* LPUART_STAT bits */
#define LPUART_STAT_TDRE    (1 << 23)   /* Transmit Data Register Empty */
#define LPUART_STAT_TC      (1 << 22)   /* Transmission Complete */
#define LPUART_STAT_RDRF    (1 << 21)   /* Receive Data Register Full */
#define LPUART_STAT_IDLE    (1 << 20)   /* Idle Line Flag */
#define LPUART_STAT_OR      (1 << 19)   /* Receiver Overrun */
#define LPUART_STAT_NF      (1 << 18)   /* Noise Flag */
#define LPUART_STAT_FE      (1 << 17)   /* Framing Error */
#define LPUART_STAT_PF      (1 << 16)   /* Parity Error */

/* LPUART_CTRL bits */
#define LPUART_CTRL_RE      (1 << 18)   /* Receiver Enable */
#define LPUART_CTRL_TE      (1 << 19)   /* Transmitter Enable */
#define LPUART_CTRL_RIE     (1 << 21)   /* Receive Interrupt Enable */
#define LPUART_CTRL_TIE     (1 << 23)   /* Transmit Interrupt Enable */

/* LPUART_FIFO bits */
#define LPUART_FIFO_RXFE    (1 << 3)    /* Receive FIFO Enable */
#define LPUART_FIFO_TXFE    (1 << 7)    /* Transmit FIFO Enable */

struct S32K344LPUARTState {
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    CharBackend chr;
    qemu_irq irq;

    uint32_t regs[16];  /* Register space */
    
    /* FIFO simulation */
    uint8_t tx_fifo[16];
    uint8_t rx_fifo[16];
    uint8_t tx_fifo_len;
    uint8_t rx_fifo_len;
    uint8_t tx_fifo_head;
    uint8_t rx_fifo_head;
};

#endif /* S32K344_LPUART_H */ 