/**
 * @file boot_start.S
 * @brief Entry point for Bootloader (NOR Flash Boot)
 * @details The bootloader runs from NOR Flash at address 0.
 *          It initializes hardware, then loads the application from NAND to SDRAM.
 *          IRQ is redirected to the application's handler in SDRAM.
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

    /* 3. Setup Stacks in Internal SRAM (4KB at 0x40000000) */
    /* Enter IRQ mode, disable IRQ/FIQ */
    msr cpsr_c, #0xd2
    ldr sp, =0x40001000    /* Top 1KB for IRQ stack */

    /* Enter SVC mode, disable IRQ/FIQ */
    msr cpsr_c, #0xd3
    ldr sp, =0x40000c00    /* 3KB for SVC stack */

    /* 4. Clear BSS */
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0
bss_loop:
    cmp r0, r1
    strlo r2, [r0], #4
    blo bss_loop

    /* 5. Jump to C logic */
    bl main

halt:
    b halt
