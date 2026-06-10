# JZ2440 Unified SDK Application Makefile
# Usage: make [V=1] [clean|all|flash|openocd|gdb]
#         make RAM_TARGET=isram  # Build for IRAM (0x40000000)

TARGET     := $(notdir $(CURDIR))
TOP_DIR   := ../../..

ifeq ($(RAM_TARGET),isram)
    # Build for IRAM (0x40000000) - all sections in internal SRAM
    LDSCRIPT   := $(TOP_DIR)/common/lds/app.lds
else
    # Default: Build for NOR Flash (0x00000000) - text/rodata in Flash, data/bss in SRAM
    RAM_TARGET := nor
    TEXT_BASE  := 0
    START_FILE := $(TOP_DIR)/common/arch/demo_start.S
    LDSCRIPT   := $(TOP_DIR)/common/lds/demo.lds
endif

include $(TOP_DIR)/common.mk
