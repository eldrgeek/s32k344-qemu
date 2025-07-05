/*
 * S32K344 Linker Script for QEMU Demo Firmware
 *
 * Memory layout for NXP S32K344:
 * - Flash: 4MB at 0x00400000 
 * - SRAM:  512KB at 0x20000000
 * - DTCM:  128KB at 0x20000000 (overlapped with SRAM base)
 * - ITCM:  64KB at 0x00000000
 */

MEMORY
{
    FLASH (rx)      : ORIGIN = 0x00400000, LENGTH = 4M
    SRAM (rwx)      : ORIGIN = 0x20000000, LENGTH = 512K
    DTCM (rwx)      : ORIGIN = 0x20000000, LENGTH = 128K
    ITCM (rwx)      : ORIGIN = 0x00000000, LENGTH = 64K
}

/* Entry point */
ENTRY(system_init)

/* Stack size */
STACK_SIZE = 0x2000;

SECTIONS
{
    /* Vector table must be at the beginning of flash */
    .vectors :
    {
        KEEP(*(.vectors))
        . = ALIGN(4);
    } > FLASH

    /* Program code and constants */
    .text :
    {
        . = ALIGN(4);
        _stext = .;
        
        *(.text)
        *(.text*)
        
        /* Read-only data */
        *(.rodata)
        *(.rodata*)
        
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    /* Exception handling frames */
    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > FLASH

    /* Initialized data (copied from flash to RAM at startup) */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        
        *(.data)
        *(.data*)
        
        . = ALIGN(4);
        _edata = .;
    } > SRAM AT > FLASH
    
    /* Store the flash address of .data section */
    _sidata = LOADADDR(.data);

    /* Zero-initialized data */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        
        *(.bss)
        *(.bss*)
        *(COMMON)
        
        . = ALIGN(4);
        _ebss = .;
    } > SRAM

    /* Stack (grows downward from top of SRAM) */
    .stack :
    {
        . = ALIGN(8);
        _sstack = .;
        . = . + STACK_SIZE;
        . = ALIGN(8);
        _estack = .;
    } > SRAM

    /* Heap (optional, between BSS and stack) */
    .heap :
    {
        . = ALIGN(4);
        _sheap = .;
        /* Heap grows up to stack */
        _eheap = _sstack;
    } > SRAM

    /* Debug sections */
    .stab 0 : { *(.stab) }
    .stabstr 0 : { *(.stabstr) }
    .stab.excl 0 : { *(.stab.excl) }
    .stab.exclstr 0 : { *(.stab.exclstr) }
    .stab.index 0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment 0 : { *(.comment) }
    
    /* DWARF debug sections */
    .debug_abbrev 0 : { *(.debug_abbrev) }
    .debug_info 0 : { *(.debug_info) }
    .debug_line 0 : { *(.debug_line) }
    .debug_frame 0 : { *(.debug_frame) }
    .debug_str 0 : { *(.debug_str) }
    .debug_loc 0 : { *(.debug_loc) }
    .debug_macinfo 0 : { *(.debug_macinfo) }
    .debug_ranges 0 : { *(.debug_ranges) }
    
    /* Discard these sections */
    /DISCARD/ :
    {
        *(.ARM.attributes)
        *(.note.GNU-stack)
        *(.gnu*)
    }
}

/* Provide symbols for memory regions */
_flash_start = ORIGIN(FLASH);
_flash_size = LENGTH(FLASH);
_sram_start = ORIGIN(SRAM);
_sram_size = LENGTH(SRAM);

/* Check for sufficient memory */
ASSERT(_estack <= (ORIGIN(SRAM) + LENGTH(SRAM)), "Stack overflow: not enough SRAM memory")
ASSERT(_etext <= (ORIGIN(FLASH) + LENGTH(FLASH)), "Program too large: not enough Flash memory") 