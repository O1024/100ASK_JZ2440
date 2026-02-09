.text
.global _start

_start:
	b reset
	b .
	b .
	b .
	b .
	b .
	b .
	b .

reset:
	/* 1. Watchdog */
	ldr r0, =0x53000000
	mov r1, #0
	str r1, [r0]

	/* 2. Clock */
	ldr r0, =0x4C000000
	ldr r1, =0xFFFFFFFF
	str r1, [r0]
	ldr r0, =0x4C000014
	mov r1, #5
	str r1, [r0]
	mrc p15, 0, r1, c1, c0, 0
	orr r1, r1, #0xc0000000
	mcr p15, 0, r1, c1, c0, 0
	ldr r0, =0x4C000004
	ldr r1, =( (92<<12) | (1<<4) | (1<<0) )
	str r1, [r0]

	/* 3. SDRAM */
	bl sdram_init

	/* 4. NAND Init & Relocate */
	bl nand_init
	
	/* 搬运代码: 从 NAND 0 地址搬运 256KB 到 SDRAM 0x30000000 */
	mov r0, #0              @ Src: NAND addr 0
	ldr r1, =_start         @ Dst: SDRAM 0x30000000
	mov r2, #0x40000        @ Len: 256KB (足够覆盖本程序)
	bl nand_read

	/* 5. Clear BSS */
	ldr r0, =__bss_start
	ldr r1, =__bss_end
	mov r2, #0
clean_bss:
	cmp r0, r1
	beq run_main
	str r2, [r0], #4
	b clean_bss

run_main:
	ldr sp, =0x33F00000
	ldr pc, =main

sdram_init:
	ldr r0, =0x48000000
	adr r1, sdram_config
	add r3, r0, #(13*4)
1:
	ldr r2, [r1], #4
	str r2, [r0], #4
	cmp r0, r3
	bne 1b
	mov pc, lr

.align 4
sdram_config:
	.word 0x22011110, 0x00000700, 0x00000700, 0x00000700
	.word 0x00000700, 0x00000700, 0x00000700, 0x00018001
	.word 0x00018001, 0x008C04F4, 0x000000B1, 0x00000030, 0x00000030

/* --- NAND Flash Driver (Assembly) --- */
#define NFCONF 0x4E000000
#define NFCONT 0x4E000004
#define NFCMMD 0x4E000008
#define NFADDR 0x4E00000C
#define NFDATA 0x4E000010
#define NFSTAT 0x4E000020

nand_init:
	ldr r0, =NFCONF
	/* TACLS=1, TWRPH0=2, TWRPH1=1 (Duration based on HCLK=100MHz) */
	ldr r1, =( (1<<12) | (2<<8) | (1<<4) )
	str r1, [r0]
	
	ldr r0, =NFCONT
	/* Enable Controller, Disable Chip Select, Init ECC */
	ldr r1, =( (1<<0) | (1<<1) | (1<<4) )
	str r1, [r0]
	mov pc, lr

nand_read:
	/* r0: NAND Start Addr, r1: SDRAM Dest Addr, r2: Length */
	mov r3, lr          @ Save return address
	
	/* Select Chip */
	ldr r4, =NFCONT
	ldr r5, [r4]
	bic r5, r5, #(1<<1)
	str r5, [r4]

nand_read_loop:
	/* Check if length is 0 */
	cmp r2, #0
	beq nand_read_end

	/* Send Read Cmd 0x00 */
	ldr r4, =NFCMMD
	mov r5, #0x00
	str r5, [r4]

	/* Send Address (5 Cycles) */
	ldr r4, =NFADDR
	
	/* Col Addr 1: A[7:0] */
	mov r5, r0
	and r5, r5, #0xFF
	str r5, [r4]
	
	/* Col Addr 2: A[10:8] */
	mov r5, r0, lsr #8
	and r5, r5, #0x07
	str r5, [r4]
	
	/* Row Addr 1: A[18:11] */
	mov r5, r0, lsr #11
	and r5, r5, #0xFF
	str r5, [r4]
	
	/* Row Addr 2: A[26:19] */
	mov r5, r0, lsr #19
	and r5, r5, #0xFF
	str r5, [r4]
	
	/* Row Addr 3: A[27] */
	mov r5, r0, lsr #27
	and r5, r5, #0x01
	str r5, [r4]

	/* Send Read Confirm 0x30 */
	ldr r4, =NFCMMD
	mov r5, #0x30
	str r5, [r4]

	/* Wait for Ready */
	ldr r4, =NFSTAT
wait_ready:
	ldr r5, [r4]
	tst r5, #1
	beq wait_ready

	/* Read Page (2048 bytes) */
	ldr r4, =NFDATA
	mov r6, #2048
read_page_loop:
	ldr r5, [r4]
	str r5, [r1], #4
	subs r6, r6, #4
	bne read_page_loop

	/* Update pointers */
	add r0, r0, #2048
	subs r2, r2, #2048
	b nand_read_loop

nand_read_end:
	/* Deselect Chip */
	ldr r4, =NFCONT
	ldr r5, [r4]
	orr r5, r5, #(1<<1)
	str r5, [r4]
	
	mov pc, r3