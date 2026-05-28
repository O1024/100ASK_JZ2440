/**
 * @file boot.lds
 * @brief Linker Script for Bootloader (NOR Flash Boot)
 * 
 * The bootloader runs from NOR Flash at address 0.
 * Code and rodata are in Flash, data and bss in internal SRAM.
 * Includes a 16KB size limit assertion for the bootloader.
 */

ENTRY(_start)

SECTIONS {
    . = 0x00000000;

    .text : {
        *(.text .text.*)
    }

    .rodata : { 
        . = ALIGN(4);
        *(.rodata .rodata.*) 
    }

    . = ALIGN(4);
    __data_flash_start = .;

    /* Data and BSS go to RAM (0x40000000) */
    .data 0x40000000 : AT(__data_flash_start) {
        __data_start = .;
        *(.data .data.*)
        . = ALIGN(4);
        __data_end = .;
    }

    /* Force BSS VMA to follow .data in SRAM */
    . = 0x40000000 + SIZEOF(.data);
    .bss : {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    }

    /* Keep it 16KB limit in Flash */
    ASSERT(__data_flash_start <= 16384, "Error: Bootloader exceeded 16KB limit!")

    /DISCARD/ : { *(.comment) *(.note*) *(.ARM.attributes) }
}
