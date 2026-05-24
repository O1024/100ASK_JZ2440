# common.mk - Unified JZ2440 Build System (Refactored for NOR Boot)

V ?= 0
Q := $(if $(filter 1,$(V)),,@)
CROSS_COMPILE ?= arm-none-eabi-
CC      := $(CROSS_COMPILE)gcc
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump
SIZE    := $(CROSS_COMPILE)size

# --- Configuration ---
# RAM_TARGET: isram (0x40000000, 4KB) or sdram (0x30000000, 64MB)
RAM_TARGET ?= isram
TOP_DIR    ?= .

COMMON_DIR := $(TOP_DIR)/common
ARCH_DIR   := $(COMMON_DIR)/arch
LDS_DIR    := $(COMMON_DIR)/lds
DRIVERS_DIR := $(COMMON_DIR)/drivers
INCLUDE_DIR := $(COMMON_DIR)/include

ROM_ADDR := 0x00000000

ifeq ($(RAM_TARGET),sdram)
    RAM_ADDR  := 0x30000000
    STACK_TOP := 0x34000000
    CFLAGS    += -DTARGET_SDRAM
else
    RAM_ADDR  := 0x40000000
    STACK_TOP := 0x40001000
    CFLAGS    += -DTARGET_ISRAM
endif

# --- Sources ---
SRCS += $(ARCH_DIR)/start.S $(COMMON_DIR)/boot/relocate.c
include $(DRIVERS_DIR)/drivers.mk
SRCS += main.c

OBJS := $(addsuffix .o, $(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

# --- Compiler Flags ---
INCLUDES := -I$(INCLUDE_DIR)
CFLAGS   += $(INCLUDES) -O2 -Wall -march=armv4t -marm \
            -fno-stack-protector -ffunction-sections -fdata-sections \
            -fno-builtin -MMD -MP \
            -DTEXT_BASE=$(ROM_ADDR) \
            -DDATA_BASE=$(RAM_ADDR) \
            -DSTACK_TOP=$(STACK_TOP)

LDFLAGS  := -nostartfiles -Wl,--gc-sections -L $(LDS_DIR) -T $(LDS_DIR)/jz2440.lds \
            -Wl,--defsym=_ROM_START=$(ROM_ADDR) -Wl,--defsym=_RAM_START=$(RAM_ADDR)

.PHONY: all clean flash flash_nor flash_nand openocd gdb

all: $(TARGET).bin $(TARGET).dis
	$(Q)TEXT_SZ=$$( $(SIZE) $(TARGET).elf | awk 'NR==2 {print $$1}' ); \
	DATA_SZ=$$( $(SIZE) $(TARGET).elf | awk 'NR==2 {print $$2}' ); \
	BSS_SZ=$$( $(SIZE) $(TARGET).elf | awk 'NR==2 {print $$3}' ); \
	NOR_USE=$$(($$TEXT_SZ + $$DATA_SZ)); \
	RAM_USE=$$(($$DATA_SZ + $$BSS_SZ)); \
	if [ "$(RAM_TARGET)" = "sdram" ]; then RAM_MAX=67108864; else RAM_MAX=4096; fi; \
	RAM_PCT=$$(($$RAM_USE * 100 / $$RAM_MAX)); \
	echo "------------------------------------------------"; \
	echo "Build Success: $(TARGET).bin"; \
	echo "RAM Target:  $(RAM_TARGET)"; \
	echo "Sections:    text=$$TEXT_SZ, data=$$DATA_SZ, bss=$$BSS_SZ (bytes)"; \
	echo "NOR Usage:   $$NOR_USE bytes"; \
	echo "RAM Usage:   $$RAM_USE / $$RAM_MAX bytes ($$RAM_PCT%)"; \
	echo "------------------------------------------------"

$(TARGET).bin: $(TARGET).elf
	$(Q)$(OBJCOPY) -O binary -S $< $@

$(TARGET).elf: $(OBJS)
	@echo "  LD      $@ (using $(LDS_DIR)/jz2440.lds)"
	@if [ "$(V)" = "1" ]; then echo "  LDFLAGS: $(LDFLAGS)"; fi
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.S
	@echo "  AS      $<"
	@if [ "$(V)" = "1" ]; then echo "  CFLAGS: $(CFLAGS)"; fi
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	@echo "  CC      $<"
	@if [ "$(V)" = "1" ]; then echo "  CFLAGS: $(CFLAGS)"; fi
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET).dis: $(TARGET).elf
	$(Q)$(OBJDUMP) -D -m arm $< > $@

clean:
	$(Q)rm -rf $(OBJS) $(DEPS) *.elf *.bin *.map *.dis

openocd:
	$(Q)openocd -f $(TOP_DIR)/tools/openocd/jz2440.cfg

gdb: $(TARGET).elf
	$(Q)gdb-multiarch -x $(TOP_DIR)/tools/gdb/gdbinit $<

flash: flash_nor

flash_nor: $(TARGET).bin
	$(Q)openocd -f $(TOP_DIR)/tools/openocd/jz2440.cfg -c "init; halt; flash erase_sector 0 0 last; flash write_image $< 0; reset; exit"

flash_nand: $(TARGET).bin
	$(Q)openocd -f $(TOP_DIR)/tools/openocd/jz2440.cfg -c "init; halt; nand probe 0; nand erase 0 0 0x80000; nand write 0 $< 0; reset; exit"

-include $(DEPS)
