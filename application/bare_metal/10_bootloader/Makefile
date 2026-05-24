# JZ2440 Unified SDK Bootloader Makefile

TARGET    := bootloader
TOP_DIR   := ../../..

# Explicitly define all sources for the bootloader
# (We need the drivers because boot_main.c uses Clock, UART, NAND, etc.)
SRCS      := $(TOP_DIR)/common/boot/nor_start.S \
             $(TOP_DIR)/common/boot/boot_main.c \
             $(TOP_DIR)/common/boot/relocate.c \
             $(TOP_DIR)/common/drivers/s3c2440_gpio.c \
             $(TOP_DIR)/common/drivers/s3c2440_clock.c \
             $(TOP_DIR)/common/drivers/s3c2440_uart.c \
             $(TOP_DIR)/common/drivers/s3c2440_nand.c \
             $(TOP_DIR)/common/drivers/s3c2440_sdram.c \
             $(TOP_DIR)/common/drivers/s3c2440_timer.c \
             $(TOP_DIR)/common/drivers/s3c2440_delay.c \
             $(TOP_DIR)/common/lib/crc16.c \
             $(TOP_DIR)/common/lib/ymodem.c \
             main.c

# Custom Linker Script for Bootloader
LDFLAGS   := -nostartfiles -Wl,--gc-sections -T $(TOP_DIR)/common/boot/nor.lds

# This is a Stage 1/2 loader, it runs from address 0
ROM_ADDR  := 0x00000000
RAM_TARGET := isram

include $(TOP_DIR)/common.mk
