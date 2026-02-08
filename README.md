# 实验四：系统时钟（PLL）配置

本实验的目标是掌握 S3C2440 的时钟体系结构，通过配置 MPLL（主锁相环）将开发板的工作频率从默认的 12MHz 提升至 400MHz，并设置合理的总线分频比。

---

## 🎯 实验目标
1.  理解 S3C2440 的时钟体系：FCLK、HCLK 和 PCLK。
2.  掌握 **MPLL** 的配置公式与参数计算。
3.  学习 **CLKDIVN** 分频寄存器的设置。
4.  理解 CPU 异步模式（Asynchronous Mode）的切换。
5.  通过流水灯速度的变化验证频率提升效果。

---

## 📚 硬件原理剖析

### 1. 时钟基准
JZ2440 外部接有一个 **12MHz** 的晶振 (Fin)。系统启动时，若不配置 PLL，CPU 将直接运行在此低频下。

### 2. 三大核心时钟
*   **FCLK (400MHz)**：用于 CPU 核。
*   **HCLK (100MHz)**：用于 AHB 总线外设（如存储控制器、中断控制器、DMA、LCD）。
*   **PCLK (50MHz)**：用于 APB 总线外设（如 UART、GPIO、Watchdog、Timer）。

### 3. 配置公式
MPLL 输出频率公式：
`Mpll = (2 * m * Fin) / (p * 2^s)`
*   `m = (MDIV + 8)`
*   `p = (PDIV + 2)`
*   `s = SDIV`

本实验设定 MDIV=92, PDIV=1, SDIV=1，计算结果为 400MHz。

---

## 💻 软件实现细节

### 1. 配置流程
必须按照严格的顺序执行，否则系统可能挂起：
1.  **设置 LOCKTIME (0x4C000000)**：设置 PLL 锁定的等待时间。
2.  **设置分频比 CLKDIVN (0x4C000014)**：设置 FCLK:HCLK:PCLK = 1:4:8。
3.  **切换 CPU 到异步模式**：若 FCLK != HCLK，必须在 `start.S` 或 C 代码中通过协处理器指令将 CPU 设为异步总线模式。
4.  **设置 MPLLCON (0x4C000004)**：最后写入 MDIV/PDIV/SDIV，系统开始升频。

### 2. 协处理器切换指令 (ARM 核心要求)
```armasm
/* 如果 HDIVN 不为 0，CPU 必须设为异步模式 */
mrc p15, 0, r1, c1, c0, 0
orr r1, r1, #0xc0000000
mcr p15, 0, r1, c1, c0, 0
```

---

## 🚀 编译与调试
1.  **编译**：`make clean && make`。
2.  **启动网关**：`openocd -f scripts/jz2440.cfg`。
3.  **下载运行**：`gdb-multiarch jz2440.elf -x scripts/gdbinit`。

**效果验证**：
观察流水灯的闪烁速度。由于主频从 12MHz 提升到了 400MHz（提升约 33 倍），在同样的延时计数值下，灯的跳变速度会变得极快。

---

## 📝 备注
*   **寄存器参考**：`include/s3c2440_soc.h` 中已补充 `LOCKTIME`, `MPLLCON`, `CLKDIVN` 等定义。
*   **维护规范**：本分支专注于时钟系统调优，严禁合并至 `main` 分支。