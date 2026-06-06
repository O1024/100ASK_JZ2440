# common/bsp/jz2440/bsp.mk - JZ2440 BSP Module List
#
# Board Support Package for 100ASK JZ2440 Development Board

BSP_DIR := $(TOP_DIR)/common/bsp/jz2440

SRCS += $(BSP_DIR)/bsp_init.c

# BSP 头文件路径
INCLUDES += -I$(BSP_DIR)
