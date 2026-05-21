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

# Get libgcc.a path automatically
LIBGCC  := $(shell $(CC) -print-libgcc-file-name)

# --- Boot Media ---
export BOOT_MEDIA ?= nand

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
ifeq ($(BOOT_MEDIA),nor)
  SPL_SRCS := $(SPL_DIR)/spl_nor_start.S \
              $(SPL_DIR)/spl_nor_main.c \
              $(COMMON_DIR)/drivers/s3c2440_clock.c \
              $(COMMON_DIR)/drivers/s3c2440_sdram.c \
              $(COMMON_DIR)/drivers/s3c2440_uart.c
  SPL_LDSCRIPT := $(SPL_DIR)/spl_nor.lds
else
  SPL_SRCS := $(SPL_DIR)/spl_start.S \
              $(SPL_DIR)/spl_main.c \
              $(COMMON_DIR)/drivers/s3c2440_clock.c \
              $(COMMON_DIR)/drivers/s3c2440_sdram.c \
              $(COMMON_DIR)/drivers/s3c2440_uart.c \
              $(COMMON_DIR)/drivers/s3c2440_nand.c
  SPL_LDSCRIPT := $(SPL_DIR)/spl.lds
endif

# Isolate SPL object files to prevent collisions with main app objects
SPL_OBJS := $(patsubst $(TOP_DIR)/%, spl_obj/%, $(addsuffix .o, $(basename $(SPL_SRCS))))

# --- Compiler Flags ---
INCLUDES := -I$(COMMON_DIR)/include
CFLAGS   += $(INCLUDES) -O2 -Wall -march=armv4t -marm
CFLAGS   += -fno-stack-protector -ffunction-sections -fdata-sections
CFLAGS   += -fno-builtin
CFLAGS   += -MMD -MP

# --- Linker Flags & TEXT_BASE ---
TEXT_BASE ?= 0x00000000
LDSCRIPT     := $(COMMON_DIR)/jz2440.lds

# Detect if we need SPL (If TEXT_BASE is 0x30000000)
ifeq ($(shell printf "%d" $(TEXT_BASE) 2>/dev/null || echo 0), 805306368)
  BUILD_SPL := 1
endif

.PHONY: all clean flash flash_nor openocd gdb

# --- Build Targets ---
all: $(TARGET).bin $(TARGET).dis
	@echo "Build Complete: $(TARGET).bin (SPL: $(if $(BUILD_SPL),YES,NO), Media: $(BOOT_MEDIA))"

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
	$(Q)$(LD) -T $(LDSCRIPT) -Ttext $(TEXT_BASE) -nostdlib --gc-sections -o $@ $^ $(LIBGCC)

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
	$(Q)$(LD) -T $(SPL_LDSCRIPT) -nostdlib --gc-sections -o $@ $^ $(LIBGCC)

# Special rule for SPL objects to keep them isolated
spl_obj/%.o: $(TOP_DIR)/%.S
	@mkdir -p $(dir $@)
	@echo "  AS (SPL) $<"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

spl_obj/%.o: $(TOP_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "  CC (SPL) $<"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

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
	$(Q)rm -rf $(OBJS) $(DEPS) $(SPL_OBJS) spl_obj *.elf *.bin *.map *.dis

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

flash_nor: 
	$(MAKE) BOOT_MEDIA=nor $(TARGET).bin
	$(Q)openocd -f $(TOOLS_DIR)/openocd/jz2440.cfg \
		-c "init; halt; flash protect 0 0 last off" \
		-c "flash erase_sector 0 0 last" \
		-c "flash write_image $(TARGET).bin 0" \
		-c "reset; exit"
