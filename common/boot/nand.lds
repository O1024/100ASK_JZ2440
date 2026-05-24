/**
 * @file nand.lds
 * @brief Linker Script for NAND SPL
 */

ENTRY(_start)

SECTIONS
{
    . = 0x00000000;

    .text : {
        *nand_start.o(.text)
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

    /* 4KB SRAM Limit for NAND Steppingstone */
    ASSERT(. <= 3584, "Error: NAND SPL exceeded 3.5KB limit!")
}
