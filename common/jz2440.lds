/**
 * @file jz2440.lds
 * @brief Unified Linker Script for JZ2440 (Supports SRAM and SDRAM)
 */

ENTRY(_start)

SECTIONS
{
    /* . will be set by -Ttext in LDFLAGS, default to 0x00000000 */

    .text : {
        *s3c2440_start.o(.text)
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
