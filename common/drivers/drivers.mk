# common/drivers/drivers.mk - Driver Module List
# 
# Optimization: All drivers are added to SRCS. 
# The linker will automatically discard unused functions via --gc-sections.

DRV_DIR := $(TOP_DIR)/common/drivers

SRCS += $(DRV_DIR)/s3c2440_gpio.c
SRCS += $(DRV_DIR)/s3c2440_clock.c
SRCS += $(DRV_DIR)/s3c2440_uart.c
SRCS += $(DRV_DIR)/s3c2440_nand.c
SRCS += $(DRV_DIR)/s3c2440_eth.c
SRCS += $(DRV_DIR)/s3c2440_sdram.c
SRCS += $(DRV_DIR)/s3c2440_lcd.c
SRCS += $(DRV_DIR)/s3c2440_irq.c
SRCS += $(DRV_DIR)/s3c2440_timer.c
SRCS += $(DRV_DIR)/s3c2440_delay.c
