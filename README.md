# 实验四：DMA 内存拷贝 (Memcpy)

本实验旨在探索 S3C2440 的直接存储器访问 (DMA) 控制器。我们将实现从一段 SDRAM 内存到另一段 SDRAM 内存的高速数据传输，并对比 CPU 软件拷贝 (`memcpy`) 与硬件 DMA 拷贝的效率差异。

---

## 🎯 实验目标
1.  **理解 DMA 原理**：学习 DMA 控制器如何在不占用 CPU 的情况下接管总线进行数据传输。
2.  **配置 DMA 寄存器**：掌握 `DISRC`, `DIDST`, `DCON` 等核心寄存器的配置方法。
3.  **实现内存到内存传输**：编写代码使用 DMA 通道 0 将源缓冲区数据拷贝到目的缓冲区。
4.  **性能对比**：通过翻转 GPIO 或定时器测量，直观感受 DMA 相比 CPU 轮询拷贝的巨大性能优势。

---

## 📚 硬件原理剖析

### 1. DMA 控制器概览
S3C2440 拥有 4 个 DMA 通道 (Ch0 ~ Ch3)。每个通道可以处理以下类型的传输：
*   **源与目的**：系统总线 (AHB) 或外设总线 (APB)。本实验中，SDRAM 到 SDRAM 均在 AHB 总线上。
*   **触发源**：硬件请求（如 UART, SPI, Timer）或软件请求。本实验使用 **软件触发 (SW Trigger)**。
*   **传输模式**：单次传输 (Single) 或 突发传输 (Burst)。SDRAM 支持 4 字突发，效率最高。

### 2. 关键寄存器 (以 Ch0 为例)
*   **DISRC0**: 源地址 (Source Address)。
*   **DISRCC0**: 源控制 (自增/固定，AHB/APB)。
*   **DIDST0**: 目的地址 (Destination Address)。
*   **DIDSTC0**: 目的控制 (自增/固定，AHB/APB)。
*   **DCON0**: 控制寄存器。
    *   **TC (Transfer Count)**: 传输字节数 = TC * 数据宽度。
    *   **DSZ**: 数据宽度 (Byte/Half-word/Word)。
    *   **TSZ**: 突发模式 (Burst4) 或 单次 (Unit)。
    *   **SWHW_SEL**: 选择软件触发。
*   **DMASKTRIG0**: 启动 DMA。

---

## 💻 软件实现规划

### 1. 场景设计
*   **源地址**：`0x30100000` (一段未使用的 SDRAM)。
*   **目的地址**：`0x30200000`。
*   **数据量**：大块数据（例如 1MB），以便观察时间差异。

### 2. 代码流程 (`src/main.c`)
1.  **环境准备**：初始化源数据（填充特定 pattern），清空目的数据。
2.  **CPU 拷贝测试**：
    *   点亮 LED1。
    *   执行 C 语言 `memcpy` 或 `for` 循环拷贝。
    *   熄灭 LED1。
3.  **DMA 拷贝测试**：
    *   配置 DMA 寄存器 (Ch0)。
    *   点亮 LED2。
    *   启动 DMA。
    *   **轮询** DMA 状态直到结束。
    *   熄灭 LED2。
4.  **校验**：比较两段内存是否一致，若一致则流水灯，否则爆闪。

---

## 🚀 编译与运行
由于我们基于 `lab/relocation`，代码将自动重定位并在 SDRAM 中全速运行，这对于准确评估 DMA 性能至关重要。

### 1. 编译
```bash
make clean && make
```

### 2. 烧录运行
```bash
# 烧录到 Nand Flash (参考上个实验)
openocd -f scripts/jz2440.cfg -c "halt; nand probe 0; nand erase 0 0 0x40000; nand write 0 jz2440.bin 0; reset; exit"
```

---

## 📝 预期结果
在传输大量数据（如 512KB 以上）时，肉眼应能观察到 DMA 拷贝对应的 LED 闪烁时间显著短于 CPU 拷贝的时间。