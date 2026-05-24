/**
 * @file nand_start.S
 * @brief Entry point for NAND Flash Boot (SPL).
 * Redirects interrupts to SDRAM application.
 */

.text
.global _start

_start:
    /* 1. Exception Vector Table */
    b reset_handler
    b . /* Undef */
    b . /* SWI */
    b . /* PAbort */
    b . /* DAbort */
    b . /* Reserved */
    /* Redirect IRQ to the Application's IRQ handler in SDRAM */
    ldr pc, =0x30000018
    b . /* FIQ */

reset_handler:
    /* 2. Disable Watchdog */
    ldr r0, =0x53000000
    mov r1, #0
    str r1, [r0]

    /* 3. Initial Stack (SRAM top @ 4096) */
    ldr sp, =4096

    /* 4. Clear BSS */
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0
bss_loop:
    cmp r0, r1
    strlo r2, [r0], #4
    blo bss_loop

    /* 5. Jump to C logic */
    bl boot_main

halt:
    b halt
