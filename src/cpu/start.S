.text
.global _start

_start:
	/* 1. 关闭看门狗 */
	ldr r0, =0x53000000
	mov r1, #0
	str r1, [r0]

	/* 2. 设置时钟频率 (FCLK=400MHz, HCLK=100MHz, PCLK=50MHz) */
	
	/* 2.1 设置 LOCKTIME */
	ldr r0, =0x4C000000
	ldr r1, =0xFFFFFFFF
	str r1, [r0]

	/* 2.2 设置分频比 CLKDIVN (FCLK:HCLK:PCLK = 1:4:8) */
	ldr r0, =0x4C000014
	mov r1, #5
	str r1, [r0]

	/* 2.3 切换 CPU 到异步模式 (若 HDIVN != 0，必须执行) */
	mrc p15, 0, r1, c1, c0, 0
	orr r1, r1, #0xc0000000
	mcr p15, 0, r1, c1, c0, 0

	/* 2.4 设置 MPLL (FCLK = 400MHz)
	 * m=92 (MDIV=92), p=1 (PDIV=1), s=1 (SDIV=1)
	 */
	ldr r0, =0x4C000004
	ldr r1, =( (92<<12) | (1<<4) | (1<<0) )
	str r1, [r0]

	/* 3. 设置栈指针 (SRAM 4KB) */
	ldr sp, =4096

	/* 4. 跳转到 main */
	bl main

halt:
	b halt