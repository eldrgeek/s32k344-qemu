/*
 * S32K344 QEMU Demo Firmware - Hello World with GPIO and UART
 *
 * This firmware demonstrates basic functionality of the S32K344 QEMU implementation:
 * - GPIO LED blinking
 * - UART serial output 
 * - Basic system initialization
 *
 * Copyright (c) 2025 QEMU S32K344 Project
 */

#include <stdint.h>
#include <stdbool.h>

/* S32K344 Memory Map */
#define SRAM_BASE           0x20000000
#define FLASH_BASE          0x00400000

/* ARM Cortex-M7 System Control Block */
#define SCB_BASE            0xE000ED00
#define SCB_VTOR            (SCB_BASE + 0x08)

/* SIUL2 GPIO Controller */
#define SIUL2_BASE          0x40290000
#define SIUL2_MSCR_BASE     (SIUL2_BASE + 0x0240)
#define SIUL2_GPDO_BASE     (SIUL2_BASE + 0x1300)
#define SIUL2_GPDI_BASE     (SIUL2_BASE + 0x1500)

/* LPUART (Low Power UART) */
#define LPUART0_BASE        0x40328000
#define LPUART_GLOBAL       (LPUART0_BASE + 0x0008)
#define LPUART_PINCFG       (LPUART0_BASE + 0x000C)
#define LPUART_BAUD         (LPUART0_BASE + 0x0010)
#define LPUART_STAT         (LPUART0_BASE + 0x0014)
#define LPUART_CTRL         (LPUART0_BASE + 0x0018)
#define LPUART_DATA         (LPUART0_BASE + 0x001C)

/* Register bit definitions */
#define SIUL2_MSCR_OBE      (1 << 21)  /* Output Buffer Enable */
#define SIUL2_MSCR_IBE      (1 << 19)  /* Input Buffer Enable */

#define LPUART_STAT_TDRE    (1 << 23)  /* Transmit Data Register Empty */
#define LPUART_STAT_TC      (1 << 22)  /* Transmission Complete */
#define LPUART_CTRL_TE      (1 << 19)  /* Transmitter Enable */
#define LPUART_CTRL_RE      (1 << 18)  /* Receiver Enable */

/* LED Configuration */
#define LED_PIN             12          /* Pin 12 for LED */
#define BUTTON_PIN          13          /* Pin 13 for button (optional) */

/* Helper macros for register access */
#define REG32(addr)         (*(volatile uint32_t *)(addr))
#define REG8(addr)          (*(volatile uint8_t *)(addr))

/* Function prototypes */
void system_init(void);
void gpio_init(void);
void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *str);
void delay(uint32_t count);
void led_on(void);
void led_off(void);
void led_toggle(void);

/* Global variables */
static volatile uint32_t tick_count = 0;

/* Vector table (minimal) */
__attribute__((section(".vectors")))
void (* const vector_table[])(void) = {
    (void (*)(void))((uint32_t)SRAM_BASE + 0x80000),  /* Initial Stack Pointer */
    (void (*)(void))system_init,                       /* Reset Handler */
    /* Additional vectors would go here */
};

/*
 * System initialization - called on reset
 */
void system_init(void)
{
    /* Set vector table offset register */
    REG32(SCB_VTOR) = FLASH_BASE;
    
    /* Initialize peripherals */
    gpio_init();
    uart_init();
    
    /* Main application loop */
    uart_puts("S32K344 QEMU Demo Started!\r\n");
    uart_puts("LED Blinky + UART Echo Test\r\n");
    uart_puts("===========================\r\n");
    
    while (1) {
        led_toggle();
        uart_puts("LED Toggle - Tick: ");
        
        /* Simple tick counter display */
        char buf[16];
        uint32_t temp = tick_count;
        int i = 0;
        if (temp == 0) {
            buf[i++] = '0';
        } else {
            while (temp > 0) {
                buf[i++] = '0' + (temp % 10);
                temp /= 10;
            }
        }
        buf[i] = '\0';
        
        /* Reverse string */
        for (int j = 0; j < i / 2; j++) {
            char tmp = buf[j];
            buf[j] = buf[i - 1 - j];
            buf[i - 1 - j] = tmp;
        }
        
        uart_puts(buf);
        uart_puts("\r\n");
        
        tick_count++;
        delay(1000000);  /* Simple delay */
    }
}

/*
 * Initialize GPIO for LED control
 */
void gpio_init(void)
{
    /* Configure LED pin as output */
    REG32(SIUL2_MSCR_BASE + (LED_PIN * 4)) = SIUL2_MSCR_OBE;
    
    /* Configure button pin as input (optional) */
    REG32(SIUL2_MSCR_BASE + (BUTTON_PIN * 4)) = SIUL2_MSCR_IBE;
    
    /* Initial LED state - off */
    led_off();
}

/*
 * Initialize UART for serial communication
 */
void uart_init(void)
{
    /* Reset LPUART */
    REG32(LPUART_GLOBAL) = 0x00000002;  /* Software reset */
    REG32(LPUART_GLOBAL) = 0x00000000;  /* Clear reset */
    
    /* Configure baud rate (simplified calculation for demo) */
    /* Assuming 40MHz clock, target 115200 baud */
    REG32(LPUART_BAUD) = 0x0F000022;  /* SBR=34, OSR=15 for ~115200 */
    
    /* Enable transmitter and receiver */
    REG32(LPUART_CTRL) = LPUART_CTRL_TE | LPUART_CTRL_RE;
}

/*
 * Send a character via UART
 */
void uart_putc(char c)
{
    /* Wait for transmit data register to be empty */
    while (!(REG32(LPUART_STAT) & LPUART_STAT_TDRE)) {
        /* Wait */
    }
    
    /* Send character */
    REG32(LPUART_DATA) = c;
    
    /* Wait for transmission to complete */
    while (!(REG32(LPUART_STAT) & LPUART_STAT_TC)) {
        /* Wait */
    }
}

/*
 * Send a string via UART
 */
void uart_puts(const char *str)
{
    while (*str) {
        uart_putc(*str++);
    }
}

/*
 * Simple delay function
 */
void delay(uint32_t count)
{
    volatile uint32_t i;
    for (i = 0; i < count; i++) {
        /* Busy wait */
    }
}

/*
 * Turn LED on
 */
void led_on(void)
{
    REG8(SIUL2_GPDO_BASE + LED_PIN) = 1;
}

/*
 * Turn LED off
 */
void led_off(void)
{
    REG8(SIUL2_GPDO_BASE + LED_PIN) = 0;
}

/*
 * Toggle LED state
 */
void led_toggle(void)
{
    static bool led_state = false;
    led_state = !led_state;
    if (led_state) {
        led_on();
    } else {
        led_off();
    }
} 