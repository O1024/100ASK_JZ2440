# JZ2440 Unified SDK - Gemini CLI 指南

本文件为 Gemini CLI 在 **JZ2440 Unified SDK** 项目中的工作指南，包含项目架构、构建系统、开发流程及编码规范。

## 🏗️ 项目架构 (Layered Architecture)

项目采用分层架构，旨在实现代码复用和模块化：

| 层级 | 目录 | 职责 |
| :--- | :--- | :--- |
| **应用层 (App)** | `application/` | 具体实验项目 (如 `bare_metal/01_led_chaser`) |
| **硬件抽象层 (HAL)** | `common/include/hal/` | 定义统一的硬件操作接口 (如 `hal_gpio.h`) |
| **底层驱动层 (LLD)** | `common/drivers/` | 基于 S3C2440 寄存器的具体驱动实现 |
| **架构层 (Arch)** | `common/arch/` | ARM920T 指令级操作及启动代码 (`s3c2440_start.S`) |
| **配置与工具 (Tools)** | `tools/` | OpenOCD 和 GDB 调试脚本 |

## 🛠️ 构建系统 (Modular Makefile)

项目使用模块化的 Makefile 系统：

1.  **`common.mk`**: 根目录下的核心构建规则，定义了编译器 (`arm-none-eabi-`)、编译选项 (`CFLAGS`)、`BOOT_MEDIA` 变量以及通用目标。
2.  **模块级 `*.mk`**: 如 `common/arch/arch.mk` 和 `common/drivers/drivers.mk`，负责向 `SRCS` 变量追加各自模块的源文件。
3.  **应用级 `Makefile`**: 位于具体的应用目录中，定义 `TARGET`、`TOP_DIR`，包含模块级 `.mk` 文件，并最终包含 `common.mk`。

### 启动介质配置
项目支持通过 `BOOT_MEDIA` 变量切换启动介质：
-   `BOOT_MEDIA=nand` (默认): 编译生成 NAND Flash 版本的 SPL。
-   `BOOT_MEDIA=nor`: 编译生成 NOR Flash 版本的 SPL。

### 常用命令
在应用目录下执行：
-   `make`: 默认编译 NAND 版本的 `.bin`。
-   `make BOOT_MEDIA=nor`: 编译 NOR 版本的 `.bin`。
-   `make clean`: 清理编译产物。
-   `make openocd`: 启动 OpenOCD 调试服务器。
-   `make gdb`: 启动 GDB 并加载 `tools/gdb/gdbinit` 进行调试。
-   `make flash`: 一键烧录到 NAND Flash。
-   `make flash_nor`: 一键烧录到 NOR Flash (会自动强制指定 `BOOT_MEDIA=nor` 进行构建)。

## 📝 编码与设计规范

### 1. 寄存器操作
-   所有寄存器定义应放在 `common/include/s3c2440_soc.h` 中。
-   使用 `volatile unsigned long *` 进行内存映射访问。

### 2. HAL 接口
-   HAL 接口声明应位于 `common/include/hal/`。
-   遵循 `hal_<module>_<action>` 的命名规范 (例: `hal_gpio_set`)。
-   应用层应**仅**调用 HAL 接口，禁止直接操作寄存器。

### 3. 驱动实现
-   驱动程序应放在 `common/drivers/`，并遵循 `s3c2440_<module>.c` 命名规则。
-   驱动程序应包含对应的 HAL 头文件，并实现其定义的接口。

### 4. 启动代码
-   `common/arch/s3c2440_start.S` 负责初始化堆栈、关闭看门狗、设置时钟，并跳转到 `main`。

## 🚀 Gemini CLI 助手指令

-   **添加新实验**: 在 `application/bare_metal/` 下创建新目录，参考 `01_led_chaser` 编写 `main.c` 和 `Makefile`。
-   **添加新驱动**: 在 `common/drivers/` 下创建 `.c` 文件，并在 `common/drivers/drivers.mk` 中注册。
-   **支持新启动介质**: 修改 `common/spl/` 下的启动汇编和 C 逻辑，并在 `common.mk` 中根据 `BOOT_MEDIA` 进行条件链接。
-   **更新构建规则**: 修改 `common.mk` 以调整全局编译参数或工具链设置。
-   **调试支持**: 如果需要适配新的仿真器，修改 `tools/openocd/jz2440.cfg`。

---
*Last Updated: 2026-05-21*
