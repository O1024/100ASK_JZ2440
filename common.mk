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
INCLUDE_DIR := $(COMMON_DIR)/include

# --- SoC Selection ---
# SOC: s3c2440 (default), or other SoC when adding new chips
SOC        ?= s3c2440
SOC_DIR    := $(COMMON_DIR)/soc/$(SOC)
DRIVERS_DIR := $(SOC_DIR)/drivers

# --- Board Selection ---
# BOARD: jz2440 (default), or other boards using the same SoC
BOARD      ?= jz2440
BOARD_DIR  := $(COMMON_DIR)/bsp/$(BOARD)

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

# Auto-detect boot media if not specified by application
ifeq ($(BOOT_MEDIA),)
    ifeq ($(RAM_TARGET),sdram)
        BOOT_MEDIA := nand
    else
        BOOT_MEDIA := nor
    endif
endif

# Allow application to select a specific start file
ifeq ($(START_FILE),)
    START_FILE := $(ARCH_DIR)/start.S
endif

# Allow application to override entire SRCS list
ifeq ($(SRCS),)
    SRCS += $(START_FILE) $(COMMON_DIR)/arch/relocate.c
    include $(DRIVERS_DIR)/drivers.mk
    include $(COMMON_DIR)/lib/lib.mk
    include $(BOARD_DIR)/bsp.mk
    SRCS += main.c
endif

OBJS := $(addsuffix .o, $(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

# --- Compiler Flags ---
INCLUDES := -I$(INCLUDE_DIR) -I$(COMMON_DIR)/include -I$(SOC_DIR)/include -I$(BOARD_DIR)
CFLAGS   += $(INCLUDES) -O2 -Wall -march=armv4t -marm \
            -fno-stack-protector -ffunction-sections -fdata-sections \
            -fno-builtin -MMD -MP \
            -DTEXT_BASE=$(ROM_ADDR) \
            -DDATA_BASE=$(RAM_ADDR) \
            -DSTACK_TOP=$(STACK_TOP)

# Allow application to select a specific linker script
ifeq ($(LDSCRIPT),)
    ifeq ($(RAM_TARGET),sdram)
        LDSCRIPT := $(LDS_DIR)/app.lds
    else
        LDSCRIPT := $(LDS_DIR)/demo.lds
    endif
endif

# Allow application to override LDFLAGS (e.g. for custom LDS)
ifeq ($(LDFLAGS),)
    ifeq ($(RAM_TARGET),sdram)
        # SDRAM apps: all sections in SDRAM, loaded by bootloader
        LDFLAGS  := -nostartfiles -Wl,--gc-sections -L$(LDS_DIR) -T$(LDSCRIPT) \
                    -Wl,--defsym=_RAM_START=$(RAM_ADDR)
    else
        # SRAM/NOR XIP apps: text/rodata in Flash, data/bss in SRAM
        LDFLAGS  := -nostartfiles -Wl,--gc-sections -L$(LDS_DIR) -T$(LDSCRIPT) \
                    -Wl,--defsym=_ROM_START=$(ROM_ADDR) -Wl,--defsym=_RAM_START=$(RAM_ADDR)
    endif
endif

.PHONY: all clean flash flash_nor flash_nand openocd gdb

all: $(TARGET).bin $(TARGET).dis
	$(Q)TEXT_SZ=$$( $(SIZE) $(TARGET).elf | awk 'NR==2 {print $$1}' ); \
	DATA_SZ=$$( $(SIZE) $(TARGET).elf | awk 'NR==2 {print $$2}' ); \
	BSS_SZ=$$( $(SIZE) $(TARGET).elf | awk 'NR==2 {print $$3}' ); \
	FLASH_SZ=$$(($$TEXT_SZ + $$DATA_SZ)); \
	if [ "$(RAM_TARGET)" = "sdram" ]; then \
		FLASH_LABEL="Image Size"; \
		RAM_USE=$$(($$TEXT_SZ + $$DATA_SZ + $$BSS_SZ)); \
		RAM_MAX=67108864; \
	else \
		FLASH_LABEL="NOR Usage"; \
		RAM_USE=$$(($$DATA_SZ + $$BSS_SZ)); \
		RAM_MAX=4096; \
	fi; \
	RAM_PCT=$$(($$RAM_USE * 100 / $$RAM_MAX)); \
	echo "------------------------------------------------"; \
	echo "Build Success: $(TARGET).bin"; \
	echo "RAM Target:  $(RAM_TARGET)"; \
	echo "Sections:    text=$$TEXT_SZ, data=$$DATA_SZ, bss=$$BSS_SZ (bytes)"; \
	echo "$$FLASH_LABEL:   $$FLASH_SZ bytes"; \
	echo "RAM Usage:   $$RAM_USE / $$RAM_MAX bytes ($$RAM_PCT%)"; \
	echo "------------------------------------------------"

$(TARGET).bin: $(TARGET).elf
	$(Q)$(OBJCOPY) -O binary -S $< $@

$(TARGET).elf: $(OBJS)
	@echo "  LD      $@ ($(notdir $(LDSCRIPT)), $(BOOT_MEDIA), $(RAM_TARGET)$(if $(EXTRA_RAM),+$(EXTRA_RAM)))"
	@if [ "$(V)" = "1" ]; then echo "  LDFLAGS: $(LDFLAGS)"; fi
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -Wl,-Map=$(TARGET).map -o $@ $^

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

flash: flash_$(BOOT_MEDIA)

flash_nor: $(TARGET).bin
	$(Q)openocd -f $(TOP_DIR)/tools/openocd/jz2440.cfg -c "init; halt; arm mcr 15 0 1 0 0 0x00000078; flash erase_sector 0 0 last; flash write_image $< 0; reset; exit"

flash_nand: $(TARGET).bin
	$(Q)openocd -f $(TOP_DIR)/tools/openocd/jz2440.cfg -c "init; halt; nand probe 0; nand erase 0 0 0x80000; nand write 0 $< 0; reset; exit"

-include $(DEPS)
