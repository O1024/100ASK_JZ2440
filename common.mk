# common.mk - Professional JZ2440 Unified Build Rules (Refined SPL Logic)

# --- Verbosity Control ---
ifeq ($(V),1)
  Q :=
else
  Q := @
endif

# --- Toolchain ---
CROSS_COMPILE ?= arm-none-eabi-
CC      := $(CROSS_COMPILE)gcc
LD      := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump

# --- Paths ---
COMMON_DIR := $(TOP_DIR)/common
SPL_DIR    := $(COMMON_DIR)/spl
ARCH_DIR   := $(COMMON_DIR)/arch
TOOLS_DIR  := $(TOP_DIR)/tools

# --- Stage 2: Main Application Sources ---
SRCS += main.c
include $(COMMON_DIR)/arch/arch.mk
include $(COMMON_DIR)/drivers/drivers.mk
include $(COMMON_DIR)/lib/lib.mk

OBJS := $(addsuffix .o, $(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

# --- Stage 1: SPL Sources ---
SPL_SRCS := $(SPL_DIR)/spl_start.S \
            $(SPL_DIR)/spl_main.c \
            $(COMMON_DIR)/drivers/s3c2440_clock.c \
            $(COMMON_DIR)/drivers/s3c2440_sdram.c \
            $(COMMON_DIR)/drivers/s3c2440_uart.c \
            $(COMMON_DIR)/drivers/s3c2440_nand.c

SPL_OBJS := $(addsuffix .o, $(basename $(SPL_SRCS)))

# --- Compiler Flags ---
INCLUDES := -I$(COMMON_DIR)/include
CFLAGS   += $(INCLUDES) -O2 -Wall -march=armv4t -marm
CFLAGS   += -fno-stack-protector -ffunction-sections -fdata-sections
CFLAGS   += -fno-builtin
CFLAGS   += -MMD -MP

# --- Linker Flags & TEXT_BASE ---
TEXT_BASE ?= 0x00000000
LDSCRIPT     := $(COMMON_DIR)/jz2440.lds
SPL_LDSCRIPT := $(SPL_DIR)/spl.lds

# Detect if we need SPL (If TEXT_BASE is 0x30000000)
ifeq ($(shell printf "%d" $(TEXT_BASE) 2>/dev/null || echo 0), 805306368)
  BUILD_SPL := 1
endif

.PHONY: all clean flash openocd gdb

# --- Build Targets ---
all: $(TARGET).bin $(TARGET).dis
	@echo "Build Complete: $(TARGET).bin (SPL: $(if $(BUILD_SPL),YES,NO))"

# 1. Final Binary Rule
ifeq ($(BUILD_SPL),1)
$(TARGET).bin: spl.bin app.bin
	@echo "  GEN     $@"
	$(Q)cat spl.bin app.bin > $@
else
$(TARGET).bin: $(TARGET).elf
	@echo "  OBJCOPY $@"
	$(Q)$(OBJCOPY) -O binary -S $< $@
endif

# 2. Main Application ELF (Always linked at TEXT_BASE)
$(TARGET).elf: $(OBJS)
	@echo "  LD      $@"
	$(Q)$(LD) -T $(LDSCRIPT) -Ttext $(TEXT_BASE) -nostdlib --gc-sections -o $@ $^ /usr/lib/gcc/arm-none-eabi/13.2.1/libgcc.a

# 3. Intermediate App Binary (Only for SPL mode)
app.bin: $(TARGET).elf
	$(Q)$(OBJCOPY) -O binary -S $< $@

# 4. SPL Binary (Linked at 0x0, strictly padded to 4KB)
spl.bin: spl.elf
	@echo "  OBJCOPY spl.bin"
	$(Q)$(OBJCOPY) -O binary -S $< spl.tmp.bin
	$(Q)dd if=spl.tmp.bin of=$@ bs=4096 conv=sync status=none
	$(Q)rm spl.tmp.bin

spl.elf: $(SPL_OBJS)
	@echo "  LD      spl.elf"
	$(Q)$(LD) -T $(SPL_LDSCRIPT) -nostdlib --gc-sections -o $@ $^ /usr/lib/gcc/arm-none-eabi/13.2.1/libgcc.a

# --- Generic Rules ---
%.o: %.S
	@echo "  AS      $<"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	@echo "  CC      $<"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET).dis: $(TARGET).elf
	@echo "  OBJDUMP $@"
	$(Q)$(OBJDUMP) -D -m arm $< > $@

clean:
	@echo "  CLEAN"
	$(Q)rm -f $(OBJS) $(DEPS) $(SPL_OBJS) *.elf *.bin *.map *.dis

-include $(DEPS)

# --- Tool Integration ---
openocd:
	$(Q)openocd -f $(TOOLS_DIR)/openocd/jz2440.cfg

gdb: $(TARGET).elf
	$(Q)gdb-multiarch -x $(TOOLS_DIR)/gdb/gdbinit $<

flash: $(TARGET).bin
	$(Q)openocd -f $(TOOLS_DIR)/openocd/jz2440.cfg \
		-c "init; halt; nand probe 0" \
		-c "nand erase 0 0 0x80000" \
		-c "nand write 0 $< 0" \
		-c "reset; exit"
