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

    .rodata : { *(.rodata*) }
    .data : { *(.data*) }

    .bss : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }

    /* Keep it 4KB to match the NAND application offset */
    ASSERT(. <= 4096, "Error: NOR Bootloader exceeded 4KB limit!")
}
