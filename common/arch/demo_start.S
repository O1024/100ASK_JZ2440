/**
 * @file demo_start.S
 * @brief Entry point for SRAM Demo Applications
 * @details Used by small demo programs running directly in internal SRAM.
 *          Stack is placed in the top of the 4KB internal SRAM.
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

    /* 2. Setup Stacks in Internal SRAM (4KB at 0x40000000) */
    /* Enter IRQ mode, disable IRQ/FIQ */
    msr cpsr_c, #0xd2
    ldr sp, =0x40001000    /* Top of SRAM */

    /* Enter SVC mode, disable IRQ/FIQ */
    msr cpsr_c, #0xd3
    ldr sp, =0x40000f00    /* SVC stack starts below IRQ stack (256 bytes for IRQ) */

    /* 3. Jump to main (main() will call hal_system_init at proper time) */
    bl main

halt:
    b halt

irq_handler:
    sub lr, lr, #4
    stmdb sp!, {r0-r12, lr}
    bl handle_irq
    ldmia sp!, {r0-r12, pc}^
