# JZ2440 Unified SDK Application Makefile
# Usage: make [V=1] [clean|all|flash|openocd|gdb]

TARGET     := $(notdir $(CURDIR))
TOP_DIR   := ../../..
ROM_ADDR   := 0x30000000
RAM_TARGET := sdram
START_FILE := $(TOP_DIR)/common/arch/app_start.S
LDSCRIPT   := $(TOP_DIR)/common/lds/app.lds

include $(TOP_DIR)/common.mk
