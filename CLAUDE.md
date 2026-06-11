# CLAUDE.md

本文件为 Claude Code (claude.ai/code) 在此仓库中工作时提供指导。

## 项目概述

面向 100ASK JZ2440 开发板的裸机 ARM920T (S3C2440) SDK。构建系统基于 Make，使用 `arm-none-eabi-gcc` 交叉编译。目前仅 `application/bare_metal/` 目录有实际代码；`application/freertos/`、`rt_thread/`、`bootloader/`、`kernel/`、`linux/` 均为空占位符。

## 构建系统

- **必须在应用目录内构建**，切勿在仓库根目录下执行 `make`。每个应用目录都有自己的 `Makefile`，通过 `TOP_DIR` 包含 `../../common.mk`。
- 默认静默编译，使用 `make V=1` 查看完整编译/链接命令。
- 工具链前缀：`arm-none-eabi-`。`common.mk` 硬编码了 `libgcc` 路径 `/usr/lib/gcc/arm-none-eabi/13.2.1/libgcc.a`。如果工具链版本不同，必须更新此路径，否则链接阶段会失败。
- `common.mk` 自动生成 `.d` 依赖文件，头文件修改会正确触发依赖重编。
- `--gc-sections` 始终生效，因此将所有驱动加入 `SRCS` 是安全的——链接器会自动丢弃未使用的函数。

### 应用 Makefile 模板

```makefile
TARGET     := $(notdir $(CURDIR))
TOP_DIR    := ../../..
ROM_ADDR   := 0x00000000              # SDRAM 应用改为 0x30000000
RAM_TARGET := isram                   # 或 sdram
START_FILE := $(TOP_DIR)/common/arch/demo_start.S   # 或 app_start.S / boot_start.S
LDSCRIPT   := $(TOP_DIR)/common/lds/demo.lds        # 或 app.lds / boot.lds
include $(TOP_DIR)/common.mk
```

默认会包含 `main.c`；`drivers.mk`、`lib.mk`、`bsp.mk` 会自动追加其余源文件。

### 模块化 .mk 系统

| 文件 | 作用 |
|---|---|
| `common.mk` | 核心构建引擎：编译器标志、链接参数、flash/gdb 目标 |
| `drivers.mk` | 注册所有 LL 驱动（`ll_*.c`）和 HAL 实现（`hal_*.c`）到 SRCS |
| `lib.mk` | 注册通用库（crc16、ymodem、string）到 SRCS |
| `bsp.mk` | 注册板级支持包（bsp_init.c）到 SRCS |

注意：`arch.mk` 文件存在但当前 `common.mk` 未引用，启动文件通过 `START_FILE` 变量直接在 `common.mk` 中处理。

### 启动介质配置 (BOOT_MEDIA)

`common.mk` 根据 `RAM_TARGET` 自动推导 `BOOT_MEDIA`：SDRAM → `nand`，ISRAM → `nor`。也可手动覆盖：

```bash
make BOOT_MEDIA=nor        # 强制编译 NOR 版本
make BOOT_MEDIA=nand       # 强制编译 NAND 版本
```

## 两种构建模式：ISRAM vs SDRAM

**ISRAM 模式**（`RAM_TARGET := isram`）——应用在 4 KB 内部 SRAM (`0x40000000`) 中运行。代码段和只读数据段在 NOR Flash 中就地执行 (XIP)，data/bss 段在 SRAM 中。使用 `demo.lds` 或 `boot.lds` 链接。适用于小型实验（LED、UART、时钟）和 bootloader。

**SDRAM 模式**（`RAM_TARGET := sdram`）——应用被加载到 64 MB SDRAM (`0x30000000`) 中运行。所有段均在 SDRAM 中。使用 `app.lds` 链接。需要由 bootloader 初始化 SDRAM 并加载应用。栈位于 `0x34000000`。

## 分层架构

```
应用层 (main.c)                    —— 仅调用 HAL + BSP 接口，禁止直接操作寄存器
    ↓
板级支持包 (common/bsp/jz2440/)    —— 板级布线：哪个 GPIO 是 LED、UART 波特率、初始化顺序
    ↓
硬件抽象层 (common/hal/)           —— 薄封装层；将 HAL 枚举转为 LL 原生值，调用 ll_* 函数
    ↓
底层驱动 (common/soc/s3c2440/drivers/) —— 基于结构体内存映射的寄存器级实现
    ↓
SoC 头文件 (s3c2440_soc.h)         —— 外设寄存器 typedef 结构体 + 基地址宏
```

- **HAL 头文件** (`common/include/hal/`)：定义公开 API 和 HAL 专有 `enum` 类型。命名规范：`hal_<module>_<action>`（如 `hal_gpio_set`、`hal_uart_puts`）。
- **HAL 实现** (`common/hal/`)：每个 `.c` 包含对应的 `hal/` 头文件和 `ll_*.h`，然后转发调用。**禁止**包含 `s3c2440_soc.h`。
- **LL 驱动** (`common/soc/s3c2440/drivers/`)：包含 `s3c2440_soc.h`，操作 `volatile` 结构体指针。LL 函数签名使用 `uint32_t`，不使用 HAL 枚举。命名规范：`ll_<module>_<action>`（如 `ll_gpio_set`）。
- **SoC 寄存器定义** (`common/soc/s3c2440/include/s3c2440_soc.h`)：所有外设寄存器映射均使用 `typedef struct` + `volatile uint32_t` 字段，而非 `#define` 地址宏。提供类型安全和 IDE 自动补全。
- **BSP** (`common/bsp/jz2440/`)：介于 HAL 和应用之间，集中定义板级常量（LED 引脚、UART 波特率、LCD 分辨率等）和外设初始化顺序。

## 启动文件与链接脚本

三种启动文件对应三种链接脚本：

| 启动文件 | 链接脚本 | 栈位置 | 用途 |
|---|---|---|---|
| `demo_start.S` | `demo.lds`（代码在 NOR，数据在 ISRAM） | ISRAM `0x40000f00` | 独立小型实验 |
| `boot_start.S` | `boot.lds`（代码在 NOR，数据在 ISRAM，16KB 限制） | ISRAM（早期）→ SDRAM（初始化后） | Bootloader |
| `app_start.S` | `app.lds`（所有段均在 SDRAM） | SDRAM `0x33ff0000` | Bootloader 加载的应用 |

所有启动文件流程：关闭看门狗 → 设置栈（IRQ + SVC 模式）→ `bl main`。均包含异常向量表，支持 IRQ（调用 `handle_irq`）。

`boot_start.S` 的特殊之处：
- 在汇编中自行清零 BSS 段（不依赖 `hal_system_init`）
- IRQ 向量重定向到应用的 ISR：`ldr pc, =0x30000018`
- 使用 `boot.lds`，Flash 占用限制 16 KB（`ASSERT(__data_flash_start <= 16384)`）

`relocate.c` 提供 `hal_system_init()`：将 `.data` 段从 Flash 复制到 RAM，并清零 `.bss` 段。由 `main()` 在使用任何已初始化的全局变量之前调用。

## 常用命令

所有命令在应用目录内执行（如 `application/bare_metal/01_demo_gpio/`）：

```bash
make              # 编译 .bin + .dis，打印段大小摘要
make V=1          # 详细模式，显示完整编译/链接命令
make clean        # 清理构建产物
make flash        # 烧录到 NOR（默认）；自动启动 openocd，擦除，写入，复位
make flash_nor    # 显式烧录到 NOR Flash
make flash_nand   # 烧录到 NAND Flash（擦除前 512 KB，然后写入）
make openocd      # 终端 A：启动 OpenOCD 调试服务器
make gdb          # 终端 B：启动 gdb-multiarch，连接，加载程序，断点停在 _start
```

## 烧录与调试

- **GDB 调试流程**：终端 A 执行 `make openocd`，终端 B 执行 `make gdb`。GDB 自动加载 `tools/gdb/gdbinit`（halt → load → break `_start`）。
- **OpenOCD 配置** (`tools/openocd/jz2440.cfg`)：使用 FTDI/OpenJTAG (`vid_pid 0x1457 0x5118`)，JTAG 频率 100 kHz，`trst_only` 复位策略。MX29LV160 NOR Flash (2 MB)，S3C2440 内部 NAND 控制器。如需适配其他仿真器，修改此文件。
- **复位问题**：OpenOCD 使用 `trst_only` 复位，如果连接后无法 Halt 目标，请手动按板载 Reset 键。

## 关键文件

| 文件 | 作用 |
|---|---|
| `common.mk` | 整个构建系统；设置编译标志、链接参数、flash/gdb 目标 |
| `common/soc/s3c2440/drivers/drivers.mk` | 注册所有 LL + HAL 源文件到 SRCS |
| `common/soc/s3c2440/include/s3c2440_soc.h` | 所有外设寄存器结构体定义和基地址宏 |
| `tools/openocd/jz2440.cfg` | OpenOCD 调试/烧录配置 |
| `tools/gdb/gdbinit` | GDB 初始化脚本 |
| `.clang-format` | 代码格式化规则（基于 LLVM 风格） |

## 编码规范

- **寄存器定义**：所有寄存器必须通过 `typedef struct` + `volatile uint32_t` 字段在 `common/soc/s3c2440/include/s3c2440_soc.h` 中定义，使用 `volatile uint32_t *` 进行内存映射访问。
- **HAL 命名**：遵循 `hal_<module>_<action>`（如 `hal_gpio_set`、`hal_uart_init`）。
- **LL 驱动命名**：遵循 `ll_<module>_<action>`（如 `ll_gpio_set`、`ll_clock_init`）。
- **应用层规则**：应用代码只能调用 HAL 和 BSP 接口，**禁止**在 `main.c` 中直接操作寄存器。
- **代码格式化**：使用 `clang-format`（基于 LLVM 风格，4 空格缩进，K&R 大括号，右对齐指针 `void *ptr`）。执行 `clang-format -i <file>` 格式化单个文件。

## 添加新代码

- **新建实验**：在 `application/bare_metal/` 下创建目录，参照同类应用添加 `main.c` 和 `Makefile`。
- **新建驱动**：
  1. 在 `common/soc/s3c2440/drivers/` 添加 `ll_<module>.c`
  2. 在 `common/soc/s3c2440/include/` 添加 `ll_<module>.h`
  3. 在 `common/include/hal/` 添加 `hal_<module>.h`
  4. 在 `common/hal/` 添加 `hal_<module>.c`
  5. 在 `common/soc/s3c2440/drivers/drivers.mk` 中注册所有新 `.c` 文件
- **新建 BSP 板**：创建 `common/bsp/<board>/`，包含 `bsp_init.c`、`bsp_init.h`、`bsp.mk`。
- **更新构建规则**：修改 `common.mk` 可调整全局编译参数、工具链设置。
- **支持新启动模式**：修改 `common/arch/` 下的启动代码和 `common.mk` 中的条件编译逻辑。

## 硬件参数速查

- **SDRAM**：64 MB，起始地址 `0x30000000`（两片 HY57V561620，32 位总线）
- **NAND**：K9F2G08U0M，2 KB/页，64 页/块，2048 块，共 256 MB
- **LCD**：480×272，16 bpp RGB565，帧缓冲区地址 `0x33800000`
- **以太网**：DM9000C，基地址 `0x20000000`（index 端口 `+0x0`，data 端口 `+0x4`），当前仅支持 MAC 环回模式 (`hal_eth_init_loopback`)
- **UART**：UART0，`hal_uart_puts` 自动将 `\n` 转为 `\r\n`
- **GPIO**：采用位编码引脚定义——`GPIO_PIN(port, pin)` 将端口号编码在 [15:8] 位，引脚号编码在 [7:0] 位

## 注意事项

- Bootloader (`10_bootloader`) 在 Makefile 中手动列举所有源文件到 `SRCS`，而非使用 `drivers.mk`——因为它只需部分驱动，且必须控制在 16 KB Flash 限制内。
- `flash` 目标默认烧录 NOR Flash。对于 NAND 启动的 SDRAM 应用，使用 `make flash_nand`。
- ISRAM 仅 4 KB；SDRAM 应用**必须**设置 `RAM_TARGET := sdram` 以获得 64 MB 内存映射。
- 本项目没有单元测试。验证方式是编译 → 烧录 → 在硬件上观察运行结果。
- `GEMINI.md`、`compile_commands.json` 以及所有构建产物（`.o`、`.bin`、`.elf`、`.dis`、`.map`、`.d`）已在 `.gitignore` 中。
- 请勿向本文件添加通用软件建议或冗长教程，保持内容仅限此仓库特有信息。
