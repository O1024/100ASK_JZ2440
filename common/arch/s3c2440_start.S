/**
 * @file s3c2440_start.S
 * @brief Professional ARM920T Startup Code with Vector Table and BSS Init
 */

.text
.global _start

_start:
    /* 1. Exception Vector Table */
    b reset_handler          /* Reset */
    ldr pc, =undef_handler   /* Undefined Instruction */
    ldr pc, =swi_handler     /* Software Interrupt (SWI) */
    ldr pc, =pabt_handler    /* Prefetch Abort */
    ldr pc, =dabt_handler    /* Data Abort */
    b .                      /* Reserved */
    ldr pc, =irq_handler     /* IRQ */
    ldr pc, =fiq_handler     /* FIQ */

reset_handler:
    /* 2. Hardware Initialization */
    /* Disable Watchdog */
    ldr r0, =0x53000000
    mov r1, #0
    str r1, [r0]

    /* 3. Stack Initialization (Prepare for C) 
     * NAND Boot: internal SRAM (4KB) is mapped to 0x0
     */
    ldr sp, =4096

    /* 4. Clear BSS Section (Mandatory for C Runtime) */
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0
bss_loop:
    cmp r0, r1
    strlo r2, [r0], #4
    blo bss_loop

    /* 5. Jump to Application Entry */
    bl main

halt:
    b halt

/* Placeholder handlers for exceptions */
undef_handler: b .
swi_handler:   b .
pabt_handler:  b .
dabt_handler:  b .
irq_handler:   b .
fiq_handler:   b .
