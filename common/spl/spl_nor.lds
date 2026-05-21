/**
 * @file spl_nor.lds
 * @brief Linker Script for JZ2440 SPL (Stage 1) - NOR Version
 */

ENTRY(_start)

SECTIONS
{
    . = 0x00000000;

    .text : {
        *spl_nor_start.o(.text)
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
     * Even though NOR doesn't have a 4KB hard limit like Steppingstone,
     * we keep it at 4KB to maintain consistent application offsets (4KB).
     */
    ASSERT(. <= 4096, "Error: NOR SPL code + data + bss exceeded 4KB limit!")
}
