# common.mk - JZ2440 Unified SDK 共享构建规则
# 
# Copyright (c) 2026 JZ2440 Unified SDK Contributors
# Distributed under the MIT License.

# 1. 工具链设置
CROSS_COMPILE ?= arm-none-eabi-
CC      = $(CROSS_COMPILE)gcc
LD      = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
GDB     = gdb-multiarch

# 2. 路径定义 (依赖于调用者定义的 TOP_DIR)
COMMON_DIR = $(TOP_DIR)/common
TOOLS_DIR  = $(TOP_DIR)/tools

# 3. 调试配置
OPENOCD     = openocd
OPENOCD_CFG = $(TOOLS_DIR)/openocd/jz2440.cfg
GDBINIT     = $(TOOLS_DIR)/gdb/gdbinit

# 4. 通用编译参数
CFLAGS += -I$(COMMON_DIR)/include -O2 -Wall -march=armv4t -marm -fno-stack-protector

# 5. 通用构建目标

# 默认目标
all: $(TARGET).bin

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary -S $< $@

$(TARGET).elf: $(OBJS)
	$(LD) -Ttext 0x0 -o $@ $^

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).bin

# --- 调试与烧录目标 ---

# 启动 OpenOCD 服务
openocd:
	$(OPENOCD) -f $(OPENOCD_CFG)

# 启动 GDB 调试
gdb: $(TARGET).elf
	$(GDB) -x $(GDBINIT) $<


# 烧录到 NAND Flash 并重启运行
flash: $(TARGET).bin
	@SIZE=$$(stat -c%s $(TARGET).bin); \
	ERASE_LEN=$$(( ($$SIZE + 0x3FFFF) & ~0x3FFFF )); \
	echo "Flashing $(TARGET).bin (Size: $$SIZE bytes, Erase Length: 0x$$(printf "%x" $$ERASE_LEN))"; \
	$(OPENOCD) -f $(OPENOCD_CFG) \
		-c "init" \
		-c "halt" \
		-c "nand probe 0" \
		-c "nand erase 0 0 $$(printf "0x%x" $$ERASE_LEN)" \
		-c "nand write 0 $(TARGET).bin 0" \
		-c "reset" \
		-c "exit"


.PHONY: all clean openocd gdb flash
