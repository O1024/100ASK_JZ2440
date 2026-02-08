.text
.global _start

_start:
	/* 0x00: Reset */
	b reset
	/* 0x04: Undefined instruction */
	ldr pc, =halt
	/* 0x08: Software interrupt */
	ldr pc, =halt
	/* 0x0c: Prefetch abort */
	ldr pc, =halt
	/* 0x10: Data abort */
	ldr pc, =halt
	/* 0x14: Reserved */
	ldr pc, =halt
	/* 0x18: IRQ */
	ldr pc, =irq_handler
	/* 0x1c: FIQ */
	ldr pc, =halt

reset:
	/* 1. 关闭看门狗 */
	ldr r0, =0x53000000
	mov r1, #0
	str r1, [r0]

	/* 2. 设置各模式下的栈指针 (SRAM 4KB) */
	/* 进入 IRQ 模式 */
	msr cpsr_c, #0xd2
	ldr sp, =3072

	/* 进入管理模式 (SVC) */
	msr cpsr_c, #0xd3
	ldr sp, =4096

	/* 3. 跳转到 main */
	bl main

halt:
	b halt

.global enable_irq
enable_irq:
	mrs r0, cpsr
	bic r0, r0, #0x80
	msr cpsr_c, r0
	mov pc, lr

irq_handler:
	/* 执行中断处理前，lr 指向被中断指令的下一条指令的下二条 (PC+8)
	 * 对于 IRQ，需要返回到 lr - 4
	 */
	sub lr, lr, #4
	/* 保存现场 */
	stmdb sp!, {r0-r12, lr}

	/* 调用 C 语言的中断处理函数 */
	bl handle_irq

	/* 恢复现场并返回 */
	ldmia sp!, {r0-r12, pc}^