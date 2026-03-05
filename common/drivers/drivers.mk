# common/drivers/drivers.mk - 驱动模块清单
DRV_DIR = $(TOP_DIR)/common/drivers

# 基础驱动
SRCS += $(DRV_DIR)/s3c2440_gpio.c
SRCS += $(DRV_DIR)/s3c2440_clock.c
