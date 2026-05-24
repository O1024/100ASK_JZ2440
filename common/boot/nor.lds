/**
 * @file nor.lds
 * @brief Linker Script for NOR Loader
 */

ENTRY(_start)

SECTIONS
{
    . = 0x00000000;

    .text : {
        *nor_start.o(.text)
        *(.init.text)
        *(.text*)
    }

    .rodata : { 
        . = ALIGN(4);
        *(.rodata*) 
    }

    . = ALIGN(4);
    __data_flash_start = .;

    /* Data and BSS go to RAM (e.g. 0x40000000) */
    .data 0x40000000 : AT(__data_flash_start) {
        __data_start = .;
        *(.data*)
        . = ALIGN(4);
        __data_end = .;
    }

    .bss : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    }

    /* Keep it 16KB limit in Flash */
    ASSERT(__data_flash_start <= 16384, "Error: NOR Bootloader exceeded 16KB limit!")
}
