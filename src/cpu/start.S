.text
.global _start

_start:
	/* 1. 关闭看门狗 */
	ldr r0, =0x53000000
	mov r1, #0
	str r1, [r0]

	/* 2. 设置时钟 (FCLK=400MHz, HCLK=100MHz, PCLK=50MHz) */
	/* 2.1 设置 LOCKTIME */
	ldr r0, =0x4C000000
	ldr r1, =0xFFFFFFFF
	str r1, [r0]

	/* 2.2 设置分频比 CLKDIVN (FCLK:HCLK:PCLK = 1:4:8) */
	ldr r0, =0x4C000014
	mov r1, #5
	str r1, [r0]

	/* 2.3 切换 CPU 到异步模式 */
	mrc p15, 0, r1, c1, c0, 0
	orr r1, r1, #0xc0000000
	mcr p15, 0, r1, c1, c0, 0

	/* 2.4 设置 MPLL (FCLK = 400MHz) */
	ldr r0, =0x4C000004
	ldr r1, =( (92<<12) | (1<<4) | (1<<0) )
	str r1, [r0]

	/* 3. 初始化 SDRAM */
	bl sdram_init

	/* 4. 设置栈指针 (在 SDRAM 中设置栈) */
	ldr sp, =0x34000000

	/* 5. 跳转到 main */
	bl main

halt:
	b halt

sdram_init:
	/* 内存控制器寄存器基地址 */
	ldr r0, =0x48000000
	/* 指向配置参数表 */
	adr r1, sdram_config
	/* 循环写入 13 个寄存器 */
	add r3, r0, #(13*4)
1:
	ldr r2, [r1], #4
	str r2, [r0], #4
	cmp r0, r3
	bne 1b
	mov pc, lr

.align 4
sdram_config:
	.word 0x22011110	@ BWSCON
	.word 0x00000700	@ BANKCON0
	.word 0x00000700	@ BANKCON1
	.word 0x00000700	@ BANKCON2
	.word 0x00000700	@ BANKCON3
	.word 0x00000700	@ BANKCON4
	.word 0x00000700	@ BANKCON5
	.word 0x00018005	@ BANKCON6
	.word 0x00018005	@ BANKCON7
	.word 0x008C04F4	@ REFRESH
	.word 0x000000B1	@ BANKSIZE
	.word 0x00000030	@ MRSRB6
	.word 0x00000030	@ MRSRB7

