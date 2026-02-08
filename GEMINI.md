# GEMINI.md - 项目指南

## 项目概览
本仓库致力于 **S3C2440 (JZ2440)** 开发板的软件开发。S3C2440 是一款广受欢迎的基于 ARM920T 内核的 SoC，广泛用于学习嵌入式系统、裸机编程和嵌入式 Linux。

本项目旨在作为以下内容的开发工作空间：
- 裸机开发（驱动程序、启动加载程序）。
- Linux 内核移植或驱动程序开发。
- 根文件系统 (rootfs) 配置。
- S3C2440 的应用层编程。

## 构建与运行
由于本项目目前处于早期阶段，具体的构建指令尚未完全建立。通常情况下，S3C2440 的开发遵循以下工作流程：

### 工具链
- **已检测到：** `arm-none-eabi-gcc` (13.2.1)
- **路径：** `/usr/bin/arm-none-eabi-gcc`
- **用途：** 裸机开发、启动加载程序。
- **建议：** 若需进行 Linux 内核或应用开发，请安装 `arm-linux-gnueabi-gcc`。

### 构建命令
- **待办：** 实现 `Makefile` 或使用 `CMake` 进行构建自动化。
- 典型命令：`make`（在提供 Makefile 之后）。

### 烧录/运行
- **OpenOCD**: 本项目已配置 OpenOCD 环境，用于在线调试和烧录。
    - 配置文件：`scripts/openocd.cfg`
    - 命令：`openocd -f scripts/openocd.cfg`
- **GDB 调试**:
    - 命令：`arm-none-eabi-gdb led.elf -x scripts/gdbinit`
- **其他方法**: JZ2440 还支持通过串口使用 `dnw` 或在 U-Boot 下使用 `tftp` 烧录。

## 开发规范
- **语言：** 主要使用 C 语言和汇编语言 (ARM)。
- **风格：** 在适用情况下，遵循标准的 Linux 内核代码风格。
- **硬件文档：** 参考 S3C2440 数据手册和 JZ2440 原理图进行外设编程。
- **测试：** 在实际硬件上验证代码，或使用支持 S3C2440 的 QEMU 等模拟器。

## 关键文件与目录
- `datasheets/`：包含核心芯片手册：
    - `S3C2440A.pdf` (SoC)
    - `ARM920T TRM.pdf` (Kernel)
    - `K9F2G08U0C.pdf` (Nand Flash)
    - `EM63A165.pdf` (SDRAM)
- `schematics/`：包含 JZ2440 V3 的电路原理图及位号图，是编写驱动程序时的重要引脚参考。
- `dev_logs/`：记录开发过程中的实验结果和调试记录。
- `README.md`：项目的中文主文档，包含硬件规格和项目目标。
- `LICENSE`：MIT 开源许可。
- `GEMINI.md`：本项目在 Gemini CLI 环境下的运行指南。
