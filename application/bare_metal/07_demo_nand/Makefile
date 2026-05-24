# JZ2440 Unified SDK Application Makefile
# Usage: make [V=1] [clean|all|flash|openocd|gdb]

TARGET    := $(notdir $(CURDIR))
TOP_DIR := ../../..
TEXT_BASE := 0x30000000

# All build magic (main.c auto-inclusion, HAL/LLD linkage) happens in common.mk
include $(TOP_DIR)/common.mk
