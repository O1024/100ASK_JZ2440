/**
 * @file app.lds
 * @brief Linker Script for SDRAM Applications (Loaded by Bootloader)
 * 
 * All sections (text, rodata, data, bss) are placed in SDRAM.
 * The bootloader loads the entire .bin to SDRAM base and jumps there.
 */

ENTRY(_start)

SECTIONS {
    . = _RAM_START;

    .text : {
        *(.text .text.*)
    }

    .rodata : {
        . = ALIGN(4);
        *(.rodata .rodata.*)
    }

    . = ALIGN(4);
    __data_flash_start = .;

    /* Data segment: LMA and VMA are the same (both in SDRAM) */
    .data . : AT(__data_flash_start) {
        __data_start = .;
        *(.data .data.*)
        . = ALIGN(4);
        __data_end = .;
    }

    .bss . : {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    }

    /DISCARD/ : { *(.comment) *(.note*) *(.ARM.attributes) }
}
