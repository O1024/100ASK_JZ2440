SECTIONS {
    . = 0x30000000;

    .text : {
        src/cpu/start.o (.text)
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    __bss_start = .;
    .bss : {
        *(.bss)
        *(.COMMON)
    }
    __bss_end = .;
}
