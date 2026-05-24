# common/arch/arch.mk - Application Architecture Sources

ifeq ($(BOOT_MEDIA),nor)
  SRCS += $(COMMON_DIR)/arch/start_nor.S
else
  SRCS += $(COMMON_DIR)/arch/start_nand.S
endif
