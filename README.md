# 实验三：代码重定位 (Relocation)

本实验旨在理解 S3C2440 的代码重定位原理。我们将程序自身从内部 SRAM (0x0) 复制到初始化好的 SDRAM (0x30000000) 中运行，并验证全局变量（.data 和 .bss 段）的正确性。

---

## 🎯 实验目标
1.  **掌握链接脚本 (Linker Script)**：通过 `.lds` 文件精确控制代码段、数据段和 BSS 段的布局。
2.  **实现重定位逻辑**：编写汇编代码将程序的有效内容拷贝到 SDRAM。
3.  **BSS 段清理**：确保未初始化的全局变量在运行前被清零。
4.  **长跳转切换**：使用绝对地址跳转，实现从 SRAM 运行到 SDRAM 运行的平滑过渡。

---

## 📚 软件实现细节

### 1. 链接脚本 (`scripts/jz2440.lds`)
我们定义了链接地址为 `0x30000000`，并导出了 `__bss_start` 和 `__bss_end` 符号，用于在汇编中确定拷贝长度和清零范围。
```ld
SECTIONS {
    . = 0x30000000;
    .text : { *(.text) }
    .data : { *(.data) }
    __bss_start = .;
    .bss : { *(.bss) *(.COMMON) }
    __bss_end = .;
}
```

### 2. 启动代码 (`src/cpu/start.S`)
*   **SDRAM 初始化**：重定位前必须先配置好内存控制器。
*   **拷贝逻辑**：
    ```assembly
    mov r0, #0              /* 源地址: SRAM 0x0 */
    ldr r1, =_start         /* 目的地址: SDRAM 0x30000000 */
    ldr r2, =__bss_start    /* 结束地址: BSS 起始位置 */
    copy_loop:
        ldr r3, [r0], #4
        str r3, [r1], #4
        cmp r1, r2
        bne copy_loop
    ```
*   **跳转**：`ldr pc, =main`。此指令会读取 `main` 的绝对链接地址并赋给 PC。

---

## 💾 硬件部署 (Nand Flash 烧录)

由于本实验涉及重定位，建议将程序永久烧录至 Nand Flash 以观察完整启动过程。

### 1. OpenOCD 配置
在 `scripts/jz2440.cfg` 中需包含 Nand 驱动定义：
```tcl
nand device s3c2440_bank0 s3c2440 $_TARGETNAME
```

### 2. 烧录指令
使用 OpenOCD 单次执行模式烧写 `jz2440.bin`：
```bash
openocd -f scripts/jz2440.cfg -c "halt; nand probe 0; nand erase 0 0 0x40000; nand write 0 jz2440.bin 0; reset; exit"
```

### 3. 运行设置
*   将开发板 **OM 拨码开关** 拨至 **NAND 启动** 位置。
*   按下复位键，SoC 将自动拷贝 Nand 前 4KB 到内部 SRAM 运行，随后触发我们的重定位逻辑跳转至 SDRAM。

---

## 🚀 编译与验证

### 1. 编译
```bash
make clean && make
```

### 2. 符号表验证
执行 `arm-none-eabi-nm jz2440.elf | grep main`，应看到 `3000012c T main`。

---

## 📝 实验备注
*   **位置无关代码 (PIC)**：在跳转到 `main` 之前，代码通过相对寻址运行在 0x0 (SRAM) 中。
*   **为何重定位**：SRAM 仅 4KB，大型应用必须迁移至 SDRAM (64MB) 运行。
