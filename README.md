# JZ2440 Unified SDK

![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![Platform: S3C2440](https://img.shields.io/badge/Platform-S3C2440-blue.svg)
![Arch: ARM920T](https://img.shields.io/badge/Arch-ARM920T-red.svg)

**JZ2440 Unified SDK** 是一个专为 100ASK JZ2440 开发板设计的工业级、分层式嵌入式开发框架。它通过精妙的硬件抽象层 (HAL) 和模块化构建系统，实现了底层驱动的高度复用与应用开发的极致简化。

---

## 💎 核心价值与工程特性

-   **解耦架构 (Layered Architecture)**: 严格区分应用层 (App)、抽象层 (HAL)、底层驱动 (LLD) 与架构层 (Arch)，支持从裸机到 RTOS 的无缝迁移。
-   **专业级 HAL 设计**: 采用**位编码引脚定义 (Bit-Encoded Pin Definitions)** 和**查表驱动 (Table-Driven)** 技术，消除丑陋的 `if-else`，提升执行效率。
-   **现代构建系统**: 
    -   **极简 App Makefile**: 应用开发只需 3 行代码配置。
    -   **自动依赖跟踪**: 完善的 `.d` 文件生成，确保头文件修改后自动重编依赖项。
    -   **静默编译控制**: 默认输出高信噪比编译日志，支持 `V=1` 详细模式。
-   **精准内存映射**: 自定义 `.lds` 链接脚本，确保 `_start` 指令精确对齐 NAND 启动地址 `0x00000000`。
-   **集成化调测**: 内置一键式 OpenOCD 烧录与 GDB 自动化调试流。

---

## 🏗️ 架构概览

| 层级 | 目录 | 职责说明 |
| :--- | :--- | :--- |
| **应用层 (App)** | `application/` | 业务逻辑实现，仅调用 HAL 接口，不直接操作寄存器。 |
| **抽象层 (HAL)** | `common/include/hal/` | 定义统一的硬件操作标准 (如 `hal_gpio_set`, `hal_uart_puts`)。 |
| **驱动层 (LLD)** | `common/drivers/` | 针对 S3C2440 的具体实现，采用查表法优化寄存器访问。 |
| **架构层 (Arch)** | `common/arch/` | ARM920T 核心初始化、堆栈设置及异常向量表定义。 |
| **构建系统 (Build)** | `common.mk` | 核心构建引擎，集成了交叉工具链配置与自动化规则。 |

---

## 📂 源码组织

```text
.
├── application/            # 应用开发区
│   └── bare_metal/         # 裸机实验案例 (LED, UART, NAND, LCD...)
├── common/                 # 核心共享组件
│   ├── arch/               # CPU 级启动代码与链接脚本
│   ├── drivers/            # 基于 S3C2440 的硬件驱动实现
│   └── include/            # SDK 公共头文件
│       ├── hal/            # 硬件抽象层标准接口
│       └── s3c2440_soc.h   # 寄存器映射定义
├── tools/                  # 辅助工具配置
│   ├── openocd/            # JZ2440 调试服务器配置
│   └── gdb/                # GDB 自动化调试初始化脚本
├── common.mk               # 核心构建规则 (Dependency-aware)
└── README.md
```

---

## 🚀 快速上手

### 1. 环境准备
确保您的 PATH 环境变量中包含：
-   `arm-none-eabi-gcc`: 建议版本 10.3+
-   `openocd`: 用于烧录与 GDB 桥接
-   `gdb-multiarch`: 跨平台调试工具

### 2. 编译示例
进入任意应用目录执行 `make`。SDK 默认采用静默编译，以突出关键警告：
```bash
cd application/bare_metal/01_demo_led
make
```
*提示：使用 `make V=1` 可查看完整的编译命令。*

### 3. 部署与调试
项目已深度集成调试指令，无需手动输入复杂的 OpenOCD 命令：

-   **一键烧录 (Nand Flash)**:
    ```bash
    make flash
    ```
-   **启动 GDB 调试**:
    ```bash
    make openocd  # 终端 A: 启动服务
    make gdb      # 终端 B: 启动调试界面
    ```

---

## 📜 许可协议
本项目基于 **MIT License** 授权。
Copyright (c) 2026 JZ2440 Unified SDK Contributors.
