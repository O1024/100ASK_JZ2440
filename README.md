# 开发日志：2026-02-04

## 核心任务：SRAM 调试环境搭建及 LED 验证

### 1. 环境搭建与工具链
- **交叉编译工具链**：确定使用 `arm-none-eabi-gcc`，并补全安装了 `gdb-multiarch` 调试器。
- **构建系统**：编写 `Makefile`，支持生成 `jz2440.bin` 和带调试信息的 `jz2440.elf`。
- **调试方式**：目前仅支持通过 OpenOCD + GDB 将程序直接下载到 SRAM 运行，尚未涉及 Flash 烧录。

### 2. 调试环境调优 (OpenOCD + GDB)
- **硬件适配**：适配 OpenJTAG 调试器 (VID:PID `1457:5118`)，选用 `ftdi` 驱动。
- **配置优化**：
    - 解决了 JTAG `all ones` 物理连接报错（通过降低速率至 800kHz 并微调复位时序）。
    - 禁用不稳定的 DCC 下载模式，解决 `DCC write failed` 错误。
    - 增加 GDB `remotetimeout` 至 120s，解决通信同步引发的 `keep_alive` 报警。
    - 设置 `reset_config none`，避开硬件复位信号干扰，使 GDB `monitor reset halt` 逻辑更稳定。

---

## LED 跑马灯实验教程 (SRAM 运行版)

### 1. 硬件原理分析
参考 JZ2440 V3 原理图第 3 页：
- **引脚连接**：LED1 -> GPF4, LED2 -> GPF5, LED4 -> GPF6。
- **控制逻辑**：GPIO 输出 **低电平 (0)** 点亮 LED，**高电平 (1)** 熄灭。
- **硬件电路**：板载 10K 外部上拉电阻，故无需开启 CPU 内部上拉。

### 2. 代码实现
在 `src/main.c` 中配置 `GPFCON` 对应位为输出 (`01`)，并在循环中切换 `GPFDAT`：

```c
// 设置 GPF4/5/6 为输出
GPFCON &= ~((3 << 8) | (3 << 10) | (3 << 12));
GPFCON |=  ((1 << 8) | (1 << 10) | (1 << 12));

while (1) {
    GPFDAT = ~(1 << 4); // LED1 亮
    delay(100000);
    // ... 依次切换 LED2, LED4
}
```

### 3. 调试与运行步骤
1. **编译**：执行 `make`。
2. **启动 OpenOCD**：`openocd -f scripts/jz2440.cfg`。
3. **启动 GDB**：另开终端执行 `gdb-multiarch jz2440.elf -x scripts/gdbinit`。
4. **验证**：GDB 会自动加载程序到 0x0 地址并停在入口，输入 `continue` 后可观察到跑马灯效果。

---

## 经验总结
- **复位技巧**：若 OpenOCD 连接超时，可在启动瞬间配合按下开发板硬件复位键。
- **SRAM 局限性**：受 S3C2440 硬件限制，SRAM 调试模式仅支持 4KB 以内的二进制文件，且断电后程序不保留。

## 备注
- 硬件：JZ2440 V3 (S3C2440A) + OpenJTAG
- 软件：Ubuntu 24.04, OpenOCD 0.12.0, GDB-multiarch
