/**
 * @file start.S
 * @brief Ultra-minimal Entry Point for JZ2440 (Fixed ISRAM Stack)
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
    ldr sp, =0x40001000    /* Top 1KB for IRQ stack */

    /* Enter SVC mode, disable IRQ/FIQ */
    msr cpsr_c, #0xd3
    ldr sp, =0x40000c00    /* 3KB for SVC stack */

    /* 4. Directly jump to main */
    bl main

halt:
    b halt

irq_handler:
    sub lr, lr, #4
    stmdb sp!, {r0-r12, lr}
    bl handle_irq
    ldmia sp!, {r0-r12, pc}^
