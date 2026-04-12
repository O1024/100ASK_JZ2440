/**
 * @file spl.lds
 * @brief Linker Script for JZ2440 SPL (Stage 1)
 */

ENTRY(_start)

SECTIONS
{
    . = 0x00000000;

    .text : {
        *spl_start.o(.text)
        *(.init.text)
        *(.text*)
    }

    .rodata : { *(.rodata*) }
    .data : { *(.data*) }

    .bss : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }

    /* 
     * SPL must fit in 4KB SRAM.
     * We limit code+data+bss to 3.5KB (3584 bytes) to leave 512 bytes for the stack.
     */
    ASSERT(. <= 3584, "Error: SPL code + data + bss exceeded 3.5KB limit!")
}
