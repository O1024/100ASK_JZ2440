# 实验二：SDRAM 内存控制器初始化与测试

本实验旨在学习如何初始化 S3C2440 的内存控制器，以驱动开发板上的外部 SDRAM (EM63A165)，并实现基本的读写测试。

---

## 🎯 实验目标
1.  **时钟系统初始化**：配置 FCLK=400MHz, HCLK=100MHz, PCLK=50MHz。SDRAM 的操作依赖 HCLK。
2.  **内存控制器配置**：编写汇编代码初始化 `BWSCON` 及 `BANKCON6` 等 13 个寄存器。
3.  **SDRAM 读写验证**：在 C 语言中对 0x30000000 地址进行读写测试。
4.  **栈空间迁移**：将栈指针 (SP) 从受限的 4KB SRAM 迁移到 64MB 的 SDRAM 空间。

---

## 📚 硬件原理剖析

### 1. SDRAM 硬件连接
参考 `docs/schematics/JZ2440_V3电路图.pdf`：
*   **SoC Bank**：SDRAM 连接在 **BANK6**，起始地址为 `0x30000000`。
*   **芯片型号**：EM63A165 (16M x 16-bit)。开发板使用了两片芯片并联，构成 **32-bit** 位宽，总容量为 **64MB**。
*   **关键参数**：
    *   **Column Address**: 9-bit。
    *   **CAS Latency (CL)**: 3 (在 100MHz HCLK 下)。

### 2. 时钟频率配置
SDRAM 的性能与 HCLK 紧密相关。
*   **输入晶振**：12MHz。
*   **MPLL**：配置为 400MHz (FCLK)。
*   **分频比 (F:H:P)**：设置为 1:4:8，因此 **HCLK = 100MHz**。
*   **异步模式**：修改时钟后，必须通过协处理器指令将 CPU 切换至异步总线模式。

---

## 💻 软件实现细节

### 1. 启动代码 (`src/cpu/start.S`)
1.  **时钟初始化**：在访问 SDRAM 之前，先建立稳定的 100MHz HCLK。
2.  **SDRAM 初始化函数 (`sdram_init`)**：
    *   采用循环写入方式，将预设的 13 个配置值写入 `0x48000000` 起始的寄存器组。
    *   `REFRESH` 寄存器计算：`Refresh Count = 2^11 + 1 - 100 * 7.8125 = 1269 (0x4F5)`。
3.  **栈指针重设**：`ldr sp, =0x34000000`。将栈设置在 SDRAM 的 64MB 边界处。

### 2. SDRAM 测试逻辑 (`src/main.c`)
*   **测试函数 `sdram_test`**：
    *   向 `0x30000000` 开始的 1KB 空间写入特定序列数据 (`0x55AA55AA + i`)。
    *   读取并逐个校验。若有一位数据不匹配，则判定为失败。
*   **结果显示**：
    *   **成功**：执行正常的 LED 流水灯效果。
    *   **失败**：所有 LED 进入快速闪烁（报警状态）。

---

## 🚀 编译与调试

### 步骤 1：编译
```bash
make clean && make
```

### 步骤 2：加载并运行
```bash
# 1. 终端 A 启动 OpenOCD
openocd -f scripts/jz2440.cfg

# 2. 终端 B 使用 GDB 加载
gdb-multiarch jz2440.elf -x scripts/gdbinit
```

---

## 📝 关键寄存器快照 (sdram_config)
| 寄存器 | 配置值 | 说明 |
| :--- | :--- | :--- |
| BWSCON | 0x22011110 | BANK6: 32-bit, 使用 nBE |
| BANKCON6 | 0x00018005 | Trp=2clks, Tsrc=2clks, CL=3 |
| REFRESH | 0x008C04F4 | 刷新周期, HCLK=100MHz |
| BANKSIZE | 0x000000B1 | 64MB 容量, 开启 Burst Mode |
| MRSRB6 | 0x00000030 | CAS Latency = 3 |

---

## ⚠️ 注意事项
*   **代码运行位置**：本实验代码目前依然通过 JTAG 加载到 0x0 (SRAM) 运行，但通过指令访问了 0x30000000 处的外部内存。
*   **初始化顺序**：必须先配置时钟分频，再配置 MPLL，最后初始化内存控制器。