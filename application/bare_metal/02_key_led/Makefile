# 项目局部 Makefile (Key Control LED)
TARGET = key_led
TOP_DIR = ../../..

# 1. 实验特有的 C 源文件
SRCS = main.c

# 2. 包含架构启动代码与通用驱动
include $(TOP_DIR)/common/arch/arch.mk
include $(TOP_DIR)/common/drivers/drivers.mk

# 3. 计算所有的目标文件 (OBJS)
OBJS = $(addsuffix .o, $(basename $(SRCS)))

# 4. 包含根目录下的通用构建规则
include $(TOP_DIR)/common.mk
