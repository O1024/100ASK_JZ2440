/**
 * @file start.S
 * @brief ARM920T Initialization Code for JZ2440
 * 
 * Copyright (c) 2026 JZ2440 Unified SDK Contributors
 * Distributed under the MIT License.
 */

.text
.global _start

_start:
    /* 1. 关闭看门狗 */
    ldr r0, =0x53000000
    mov r1, #0
    str r1, [r0]

    /* 2. 设置栈 (准备调用 C 函数) 
     * 对于 JZ2440, 内部 SRAM 地址为 0x40000000, 大小 4KB
     */
    ldr sp, =0x40000000 + 4096

    /* 3. 跳转到 main */
    bl main

halt:
    b halt
