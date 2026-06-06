# JZ2440 Unified SDK Application Makefile

TARGET     := 13_app_lcd
TOP_DIR   := ../../..

# This application is designed to be loaded into SDRAM by the bootloader
ROM_ADDR    := 0x30000000
RAM_TARGET  := sdram
START_FILE  := $(TOP_DIR)/common/arch/app_start.S
LDSCRIPT    := $(TOP_DIR)/common/lds/app.lds

include $(TOP_DIR)/common.mk
