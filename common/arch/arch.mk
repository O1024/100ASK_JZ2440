# common/arch/arch.mk - Application Architecture Sources
# 
# Available start files:
#   demo_start.S  - For SRAM demo applications (default)
#   boot_start.S  - For bootloader (NOR boot)
#   app_start.S   - For SDRAM applications (loaded by bootloader)
#
# Usage in application Makefile:
#   START_FILE := $(TOP_DIR)/common/arch/demo_start.S

# Default start file (maintains backward compatibility)
ifeq ($(START_FILE),)
  START_FILE := $(COMMON_DIR)/arch/start.S
endif

SRCS += $(START_FILE)
