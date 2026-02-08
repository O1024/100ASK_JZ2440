# JZ2440 (S3C2440) 嵌入式全栈开发实验室

本项目是根据 **JZ2440 V3** 开发板原理图、**S3C2440A 数据手册**以及**《嵌入式Linux应用开发完全手册》**定制的深度学习路线图。课程涵盖了从裸机外设控制，到 Bootloader、Linux 内核移植及 GUI 应用开发的完整全栈流程。

本项目采用“一实验一分支”的模式，每个分支都是一个独立的完整工程。

---

## 🗺️ 第一阶段：基础裸机与核心外设 (Basic Bare-metal)
*掌握最基础的硬件控制，让板子“活”起来。*

### 1. 环境搭建与硬件初识
*   **学习目标**：搭建交叉编译环境，理解启动模式（NAND/NOR）与时钟基准。
*   **实验分支**: **[lab/led_chaser](../../tree/lab/led_chaser)** (SRAM 运行机制与流水灯)

### 2. GPIO 控制：点灯与按键
*   **学习目标**：掌握 GPIO 寄存器操作，理解内部上拉电阻与轮询机制。
*   **实验分支**: **[lab/button_led](../../tree/lab/button_led)** (轮询扫描模式)

### 3. 中断与异常体系
*   **学习目标**：深入理解 ARM 异常向量表、IRQ 模式现场保护与中断控制器配置。
*   **实验分支**: **[lab/io_interrupt](../../tree/lab/io_interrupt)** (外部中断 EINT 与 异步/同步消抖)

---

## 🗺️ 第二阶段：系统架构与低功耗管理 (System & Power)
*不仅仅是跑起来，更要跑得高效、省电。深入 SoC 核心机制。*

### 4. 时钟体系深度剖析
*   **核心内容**: MPLL/UPLL 配置公式，FCLK/HCLK/PCLK 分频策略。
*   **计划实验**: **[lab/clock]**: 将系统主频从 12MHz 提升至 400MHz。

### 5. 串口通信（UART）
*   **学习目标**：掌握 UART 协议，实现波特率计算，建立 `printf` 调试窗口。
*   **实验分支**: **[lab/uart](../../tree/lab/uart)** (UART0 基础收发与回显)

### 6. 存储控制器与代码重定位
*   **核心内容**: SDRAM 时序配置，NAND Flash 读写与 ECC 校验，代码搬运 (Relocation)。
*   **计划实验**: 
    - **[lab/sdram]**: 初始化 64MB SDRAM。
    - **[lab/relocation]**: 从 SRAM/NAND 搬运代码至 SDRAM 运行。

### 7. 电源管理 (Low Power Modes)
*   **核心内容**: Normal, Slow, Idle, Sleep 四种模式切换与唤醒机制 (Wakeup)。
*   **计划实验**: **[lab/power_manage]**: 实现空闲自动 Idle 与按键唤醒 Sleep 模式。

### 8. DMA 高速数据传输
*   **核心内容**: 握手模式 (Handshake) 与按需模式 (Demand)，解放 CPU。
*   **计划实验**: **[lab/dma_memcpy]**: 使用 DMA 进行内存拷贝并对比 CPU 拷贝效率。

---

## 🗺️ 第三阶段：系统可靠性与接口扩展 (Reliability & Expansion)
*提升系统稳定性，扩展专用硬件接口。*

### 9. PWM 定时器与看门狗
*   **核心内容**: 双缓冲 PWM 波形生成 (Dead Zone)，Watchdog 复位机制。
*   **计划实验**: 
    - **[lab/pwm_backlight]**: 通过 PWM 调节 LCD 背光亮度。
    - **[lab/watchdog]**: 模拟系统死机并自动复位。

### 10. RTC 实时时钟
*   **核心内容**: BCD 格式读写，RTC Alarm 闹钟唤醒 (Wakeup Source)。
*   **计划实验**: **[lab/rtc_alarm]**: 实现电子时钟与定时唤醒功能。

### 11. SPI 总线
*   **核心内容**: SPI 传输时序配置，查询/中断/DMA 模式。
*   **计划实验**: **[lab/spi_loopback]**: SPI 接口回环测试。

### 12. 总线优先级 (Bus Priorities)
*   **核心内容**: 总线仲裁器配置 (Fixed/Rotation)，优化高负载下的系统实时性。
*   **计划实验**: **[lab/bus_arbiter]**: 调整 LCD DMA 与 CPU 访存优先级。

---

## 🗺️ 第四阶段：复杂通讯与人机交互 (Connectivity & HMI)
*   **[lab/usb_device]**: USB HID 设备模拟。
*   **[lab/lcd]**: TFT LCD 驱动与 Framebuffer。
*   **[lab/touch_screen]**: 触摸屏 ADC 转换。
*   **[lab/camera_preview]**: 摄像头接口 (CAMIF) 视频预览。
*   **[lab/audio_player]**: IIS 音频总线播放。

---

## 🗺️ 第五阶段：高级存储与文件系统 (Advanced Storage)
*   **[lab/nand_ecc]**: 启用硬件 ECC 进行 NAND 读写校验。
*   **[lab/rootfs_build]**: 使用 Busybox 构建最小根文件系统。

---

## 🗺️ 第六阶段：Linux 系统移植与 GUI (Linux & GUI)
*   **[lab/u-boot]**: U-Boot 移植与网卡/Flash 适配。
*   **[lab/linux_kernel]**: 内核裁剪、设备树 (DTB) 配置。
*   **[lab/qt_hello]**: 移植 Qt/Embedded 或 MiniGUI。
*   **调试技能**: OpenOCD/JTAG 硬件调试，GDB Server 远程应用调试。

---

## ⚠️ 分支维护规范
1. **禁止合并**: `main` 分支仅作为课程索引，严禁将任何实验分支 (`lab/*`) 合并至主分支。
2. **README 同步**: 各分支的 `README.md` 与该阶段的开发日志实时同步。
3. **硬件参考**: 核心手册存放在 `docs/datasheets/`（仅在实验分支可见）。

