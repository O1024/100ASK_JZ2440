# JZ2440 Unified SDK

这是一个针对 **100ASK JZ2440** 开发板设计的模块化、分层式 Monorepo 开发框架。旨在实现底层驱动的一次编写、多处复用，支持从裸机开发到 RTOS 移植的平滑过渡。

## 🧱 核心架构：HAL 与模块化构建

项目通过 **HAL (硬件抽象层)** 隔离业务逻辑与底层寄存器，并采用 **模块化 Makefile** 系统实现高效的代码组织：

| 层级 | 职责 | 示例 |
| :--- | :--- | :--- |
| **App Layer** | 业务逻辑 | `application/bare_metal/01_led_chaser` |
| **HAL API** | **统一标准接口** | `common/include/hal/hal_gpio.h` |
| **LLD Driver** | 寄存器级实现 | `common/drivers/s3c2440_gpio.c` |
| **Build System** | 模块化规则 | `common.mk`, `arch.mk`, `drivers.mk` |

## 📂 目录结构

```text
jz2440_unified_sdk/
├── application/            # 【应用层】存放具体的实验项目
│   └── bare_metal/         # 裸机实验 (如 01_led_chaser)
├── common/                 # 【核心层】所有工程共享的代码
│   ├── arch/               # ARM920T 指令级操作 (start.S, arch.mk)
│   ├── include/            # 寄存器定义与 HAL 接口声明
│   │   ├── hal/            # 统一 HAL 接口 (hal_gpio.h, hal_uart.h)
│   │   └── s3c2440_soc.h   # 寄存器映射
│   └── drivers/            # 基于 HAL 接口的 S3C2440 驱动实现 (drivers.mk)
├── tools/                  # 【工具链】调试与烧录配置
│   ├── openocd/            # OpenOCD 配置文件 (jz2440.cfg)
│   └── gdb/                # GDB 自动化脚本 (gdbinit)
├── common.mk               # 【构建系统】全局通用的 Makefile 构建与调试规则
├── LICENSE                 # MIT 开源协议
└── README.md
```

## 🛠️ 快速开始

### 1. 环境准备
确保您的系统中已安装以下工具：
-   **交叉编译链**: `arm-none-eabi-gcc`
-   **调试器**: `gdb-multiarch`
-   **烧录工具**: `openocd` (支持您的调试器，如 J-Link)

### 2. 编译项目
进入应用目录，`Makefile` 会自动包含所需模块并编译：
```bash
cd application/bare_metal/01_led_chaser
make
```

### 3. 烧录与调试
根目录的 `common.mk` 已封装好常用的调试指令：

*   **启动调试服务**:
    ```bash
    make openocd
    ```
*   **一键烧录至 NAND Flash**:
    (脚本会自动计算镜像大小，并进行 256KB 对齐擦除后烧录)
    ```bash
    make flash
    ```
*   **交互式调试**:
    (自动连接并加载 `tools/gdb/gdbinit` 中的配置)
    ```bash
    make gdb
    ```

## 📜 开源协议
本项目采用 [MIT License](LICENSE) 开源协议。
Copyright (c) 2026 JZ2440 Unified SDK Contributors.
