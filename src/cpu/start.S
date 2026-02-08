.text
.global _start

_start:
	/* 1. 关闭看门狗 */
	ldr r0, =0x53000000
	mov r1, #0
	str r1, [r0]

	/* 2. 设置栈指针 (内部SRAM为4KB，地址范围0x00000000 ~ 0x00000FFF) */
	ldr sp, =4096

	/* 3. 跳转到 main */
	bl main

halt:
	b halt
