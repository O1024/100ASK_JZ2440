/**
 * @file app_start.S
 * @brief Entry point for SDRAM Applications
 * @details Used by applications loaded into SDRAM by the bootloader.
 *          Stack is placed at the top of SDRAM (0x34000000).
 *          All sections (text, rodata, data, bss) are in SDRAM.
 */

.text
.global _start

_start:
    /* 0. Exception Vector Table */
    b reset_handler
    ldr pc, =halt
    ldr pc, =halt
    ldr pc, =halt
    ldr pc, =halt
    ldr pc, =halt
    ldr pc, =irq_handler
    ldr pc, =halt

reset_handler:
    /* 1. Disable Watchdog */
    ldr r0, =0x53000000
    mov r1, #0
    str r1, [r0]

    /* 2. Setup Stacks in SDRAM (64MB, top at 0x34000000) */
    /* Enter IRQ mode, disable IRQ/FIQ */
    msr cpsr_c, #0xd2
    ldr sp, =0x34000000    /* Top of SDRAM for IRQ stack */

    /* Enter SVC mode, disable IRQ/FIQ */
    msr cpsr_c, #0xd3
    ldr sp, =0x33ff0000    /* Main SVC stack */

    /* 3. Initialize C runtime (copy .data, clear .bss) */
    bl hal_system_init

    /* 4. Jump to main */
    bl main

halt:
    b halt

irq_handler:
    sub lr, lr, #4
    stmdb sp!, {r0-r12, lr}
    bl handle_irq
    ldmia sp!, {r0-r12, pc}^
