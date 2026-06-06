# AGENTS.md — JZ2440 Unified SDK

本文件为 OpenCode 会话提供紧凑参考，帮助快速上手并避免常见错误。

## 项目概述

面向 100ASK JZ2440 开发板的裸机 ARM920T (S3C2440) SDK。构建系统基于 Make，使用 `arm-none-eabi-gcc` 交叉编译。目前只有 `application/bare_metal/` 目录有实际代码；`application/freertos/`、`rt_thread/`、`bootloader/`、`kernel/`、`linux/` 均为空占位符。

## 构建系统 (common.mk)

- **切勿在仓库根目录下构建。** 每个应用目录都有自己的 Makefile，通过 `TOP_DIR` 包含 `../../common.mk`。
- **工具链：** `arm-none-eabi-` 前缀。`common.mk` 硬编码了 `libgcc` 路径为 `/usr/lib/gcc/arm-none-eabi/13.2.1/libgcc.a`。如果工具链版本不同，必须更新此路径，否则链接阶段会失败。
- **默认静默编译。** 使用 `make V=1` 查看完整编译命令。
- **应用 Makefile 模板（3–4 行）：**
  ```makefile
  TARGET  := my_app
  TOP_DIR := ../../..
  TEXT_BASE := 0x30000000   # 可选；见下方启动模式说明
  include $(TOP_DIR)/common.mk
  ```
  默认包含 `main.c`；`arch.mk`、`drivers.mk` 和 `lib.mk` 会自动追加其余源文件。

## 启动模式：SRAM vs SDRAM / SPL

- **SRAM 模式（默认，`TEXT_BASE` 未设置或为 `0x00000000`）：** 应用链接到 `0x0`，直接在 4 KB 内部 SRAM 中运行。不生成 SPL。用于早期实验（LED、UART 等）。
- **SDRAM 模式（`TEXT_BASE := 0x30000000`）：** `common.mk` 检测到后会构建**两级二进制**：
  1. **SPL** (`common/spl/`) — 链接到 `0x0`，严格填充至 4 KB。负责初始化时钟、SDRAM、UART、NAND，然后从 NAND 偏移 4 KB 处加载主应用到 SDRAM `0x30000000` 并跳转执行。
  2. **App** — 链接到 `0x30000000`。
  最终 `.bin` 为 `cat spl.bin app.bin`。
- **SPL 大小限制：** `spl.lds` 断言 `code + data + bss <= 3584` 字节（预留 512 字节给栈）。
- **栈自动检测：** `common/arch/s3c2440_start.S` 通过判断 PC 是否大于 `0x30000000` 来决定 SP = `0x34000000`（SDRAM）还是 `4096`（SRAM）。

## 添加代码

- **新建裸机应用：** 在 `application/bare_metal/` 下创建目录，添加 `main.c` 和遵循上述模板的 `Makefile`。参考同类应用复制 `TEXT_BASE`。
- **新建驱动：** 在 `common/soc/$(SOC)/drivers/` 添加 `.c` 文件，然后在 `common/soc/$(SOC)/drivers/drivers.mk` 中注册。链接器使用 `--gc-sections`，因此将所有驱动加入 `SRCS` 是安全的。
- **新建 HAL 接口：** 在 `common/include/hal/hal_<模块>.h` 中声明，在 `common/soc/$(SOC)/drivers/s3c2440_<模块>.c` 中实现。
- **寄存器定义：** 放在 `common/soc/$(SOC)/include/s3c2440_soc.h`（基于结构体的内存映射）。
- **应用层规则：** 应用代码只能调用 HAL API (`hal_<模块>_<动作>`)。禁止在 `main.c` 中直接操作寄存器。

## 烧录与调试

- **烧录到 NAND：** 在应用目录下执行 `make flash`。先擦除 NAND 前 512 KB，再写入 `.bin`。
- **GDB 调试流程：**
  - 终端 A: `make openocd`（启动 OpenOCD 服务器）
  - 终端 B: `make gdb`（启动 `gdb-multiarch`，加载 `tools/gdb/gdbinit`）
- **OpenOCD 配置** (`tools/openocd/jz2440.cfg`) 使用 FTDI/OpenJTAG (`vid_pid 0x1457 0x5118`)，`trst_only` 复位策略。如果连接后无法 Halt，请手动按板载 Reset 键。

## 关键常量与硬件参数

- **NAND：** K9F2G08U0M — 2 KB 页，64 页/块，2048 块，共 256 MB。
- **SDRAM：** 64 MB，起始地址 `0x30000000`（HY57V561620 x2，32 位总线）。
- **LCD：** 480x272，16 bpp RGB565。帧缓冲区地址 `0x33800000`。
- **UART：** `hal_uart_puts` 自动将 `\n` 转换为 `\r\n`。
- **以太网：** DM9000C，基地址 `0x20000000`（index/data 分别在 `+0x0` / `+0x4`）。当前驱动仅支持 MAC 环回模式 (`hal_eth_init_loopback`)。

## 注意事项

- `compile_commands.json` 和 `GEMINI.md` 已在 `.gitignore` 中。
- `common.mk` 自动生成 `.d` 依赖文件，头文件修改会自动触发相关对象文件重编。
- 本仓库**没有单元测试**。验证方式是 `make` 编译后烧录到硬件运行（或配置 QEMU 模拟）。
- 请勿向本文件添加通用软件建议或冗长教程，保持内容仓库专属。
