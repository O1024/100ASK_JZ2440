# common/drivers/drivers.mk - Driver Module List
# 
# Optimization: All drivers are added to SRCS. 
# The linker will automatically discard unused functions via --gc-sections.

DRV_DIR := $(TOP_DIR)/common/soc/s3c2440/drivers
HAL_DIR := $(TOP_DIR)/common/hal

SRCS += $(DRV_DIR)/ll_gpio.c
SRCS += $(DRV_DIR)/ll_clock.c
SRCS += $(DRV_DIR)/ll_uart.c
SRCS += $(DRV_DIR)/ll_nand.c
SRCS += $(DRV_DIR)/ll_eth.c
SRCS += $(DRV_DIR)/ll_sdram.c
SRCS += $(DRV_DIR)/ll_lcd.c
SRCS += $(DRV_DIR)/ll_irq.c
SRCS += $(DRV_DIR)/ll_timer.c
SRCS += $(DRV_DIR)/ll_delay.c
SRCS += $(DRV_DIR)/ll_dma.c
SRCS += $(DRV_DIR)/ll_cache.c
SRCS += $(DRV_DIR)/ll_mmu.c

SRCS += $(HAL_DIR)/hal_uart.c
SRCS += $(HAL_DIR)/hal_clock.c
SRCS += $(HAL_DIR)/hal_gpio.c
SRCS += $(HAL_DIR)/hal_timer.c
SRCS += $(HAL_DIR)/hal_nand.c
SRCS += $(HAL_DIR)/hal_sdram.c
SRCS += $(HAL_DIR)/hal_lcd.c
SRCS += $(HAL_DIR)/hal_dma.c
SRCS += $(HAL_DIR)/hal_eth.c
SRCS += $(HAL_DIR)/hal_irq.c
SRCS += $(HAL_DIR)/hal_delay.c
SRCS += $(HAL_DIR)/hal_cache.c
SRCS += $(HAL_DIR)/hal_mmu.c
