# JZ2440 Unified SDK Bootloader Makefile

TARGET    := bootloader
TOP_DIR   := ../../..

# Explicitly define all sources for the bootloader
# (We need the drivers because boot_main.c uses Clock, UART, NAND, etc.)
SRCS      := $(TOP_DIR)/common/arch/boot_start.S \
             $(TOP_DIR)/common/arch/relocate.c \
             $(TOP_DIR)/common/soc/s3c2440/drivers/ll_gpio.c \
             $(TOP_DIR)/common/soc/s3c2440/drivers/ll_clock.c \
             $(TOP_DIR)/common/soc/s3c2440/drivers/ll_uart.c \
             $(TOP_DIR)/common/soc/s3c2440/drivers/ll_nand.c \
             $(TOP_DIR)/common/soc/s3c2440/drivers/ll_sdram.c \
             $(TOP_DIR)/common/soc/s3c2440/drivers/ll_timer.c \
             $(TOP_DIR)/common/soc/s3c2440/drivers/ll_delay.c \
             $(TOP_DIR)/common/hal/hal_gpio.c \
             $(TOP_DIR)/common/hal/hal_clock.c \
             $(TOP_DIR)/common/hal/hal_uart.c \
             $(TOP_DIR)/common/hal/hal_nand.c \
             $(TOP_DIR)/common/hal/hal_sdram.c \
             $(TOP_DIR)/common/hal/hal_timer.c \
             $(TOP_DIR)/common/hal/hal_delay.c \
             $(TOP_DIR)/common/lib/crc16.c \
             $(TOP_DIR)/common/lib/ymodem.c \
             main.c

# This is a Stage 1/2 loader, it runs from address 0
# Stack: ISRAM early, SDRAM after hal_sdram_init()
ROM_ADDR   := 0x00000000
RAM_TARGET := isram
EXTRA_RAM  := sdram
LDSCRIPT   := $(TOP_DIR)/common/lds/boot.lds

include $(TOP_DIR)/common.mk
