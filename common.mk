# common.mk - Professional JZ2440 Unified Build Rules

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
TOOLS_DIR  := $(TOP_DIR)/tools

# --- Sources & Objects ---
SRCS += main.c
include $(COMMON_DIR)/arch/arch.mk
include $(COMMON_DIR)/drivers/drivers.mk
include $(COMMON_DIR)/lib/lib.mk

OBJS := $(addsuffix .o, $(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

# --- Compiler Flags ---
INCLUDES := -I$(COMMON_DIR)/include
CFLAGS   += $(INCLUDES) -O2 -Wall -march=armv4t -marm
CFLAGS   += -fno-stack-protector -ffunction-sections -fdata-sections
CFLAGS   += -MMD -MP # Dependency generation

# --- Linker Flags ---
LDSCRIPT := $(COMMON_DIR)/jz2440.lds
LDFLAGS  += -T $(LDSCRIPT) -nostdlib /usr/lib/gcc/arm-none-eabi/13.2.1/libgcc.a -Wl,--gc-sections -Wl,-Map,$(TARGET).map

# --- Build Targets ---
.PHONY: all clean flash openocd gdb

all: $(TARGET).bin $(TARGET).dis
	@echo "Build Complete: $(TARGET).bin"

$(TARGET).bin: $(TARGET).elf
	@echo "  OBJCOPY $@"
	$(Q)$(OBJCOPY) -O binary -S $< $@

$(TARGET).dis: $(TARGET).elf
	@echo "  OBJDUMP $@"
	$(Q)$(OBJDUMP) -D -m arm $< > $@

$(TARGET).elf: $(OBJS)
	@echo "  LD      $@"
	$(Q)$(LD) -T $(LDSCRIPT) -nostdlib --gc-sections -Map $(TARGET).map -o $@ $^ /usr/lib/gcc/arm-none-eabi/13.2.1/libgcc.a

%.o: %.S
	@echo "  AS      $<"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	@echo "  CC      $<"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "  CLEAN"
	$(Q)rm -f $(OBJS) $(DEPS) $(TARGET).elf $(TARGET).bin $(TARGET).map $(TARGET).dis

-include $(DEPS)

# --- Tools Targets ---
openocd:
	$(Q)openocd -f $(TOOLS_DIR)/openocd/jz2440.cfg

gdb: $(TARGET).elf
	$(Q)gdb-multiarch -x $(TOOLS_DIR)/gdb/gdbinit $<

flash: $(TARGET).bin
	$(Q)openocd -f $(TOOLS_DIR)/openocd/jz2440.cfg \
		-c "init; halt; nand probe 0" \
		-c "nand erase 0 0 0x40000" \
		-c "nand write 0 $< 0" \
		-c "reset; exit"
