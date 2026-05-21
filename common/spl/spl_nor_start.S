/**
 * @file spl_nor_start.S
 * @brief Diagnostic Startup to identify Reset Loops
 */

.text
.global _start

_start:
    /* 1. Disable Watchdog */
    mov r0, #0x53000000
    mov r1, #0
    str r1, [r0]

    /* 2. Initial Stack (4KB Top of SRAM: 0x40001000) */
    mov sp, #0x40000000
    add sp, sp, #0x1000

    /* 3. Jump to C SPL logic */
    bl spl_nor_main

halt:
    b halt
