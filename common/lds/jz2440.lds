/**
 * @file jz2440.lds
 * @brief Professional Linker Script for NOR-Boot XIP.
 */

ENTRY(_start)

SECTIONS {
    /* Code and Read-only data in Flash */
    . = _ROM_START;
    .text : {
        *(.text .text.*)
    }
    .rodata : {
        . = ALIGN(4);
        *(.rodata .rodata.*)
    }

    . = ALIGN(4);
    _data_flash_start = .; 
    
    . = _RAM_START;

    /* Data segment: LMA is Flash, VMA is RAM (e.g., 0x40000000) */
    .data . : AT(_data_flash_start) {
        _data_start = .;
        *(.data .data.*)
        . = ALIGN(4);
        _data_end = .;
    }

    /* BSS segment: In RAM, follows .data */
    .bss . : {
        _bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    }

    /DISCARD/ : { *(.comment) *(.note*) *(.ARM.attributes) }
}
