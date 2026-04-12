/**
 * @file spl_start.S
 * @brief Professional Stage 1 Boot Entry for JZ2440
 */

.text
.global _start

_start:
    /* 1. Disable Watchdog */
    ldr r0, =0x53000000
    mov r1, #0
    str r1, [r0]

    /* 2. Initial Stack (4KB Top of SRAM) 
     * Since the SPL binary is limited to 3.5KB, 
     * this provides 512 bytes for function calls and local variables.
     */
    ldr sp, =4096

    /* 3. Clear SPL BSS section (if needed) */
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0
spl_bss_loop:
    cmp r0, r1
    strlo r2, [r0], #4
    blo spl_bss_loop

    /* 4. Jump to C SPL logic */
    bl spl_main

halt:
    b halt
