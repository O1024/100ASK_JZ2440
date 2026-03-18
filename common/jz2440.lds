/**
 * @file jz2440.lds
 * @brief Professional Linker Script for JZ2440
 */

ENTRY(_start)

SECTIONS
{
    . = 0x00000000;

    .text : {
        KEEP(*s3c2440_start.o(.text))
        *(.text .text.*)
    }

    .rodata : {
        *(.rodata .rodata.*)
    }

    .data : {
        *(.data .data.*)
    }

    .bss : {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    }

    /* Remove unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.ARM.attributes)
    }
}
