ifneq ($(filter m y,$(CONFIG_INPUT_ILI_0FLASH_MMI_ENABLE_DOUBLE_TAP)),)
	ccflags-y += -DILI_SENSOR_EN
endif
ifneq ($(filter m y,$(CONFIG_INPUT_ILI_PANEL_NOTIFICATIONS)),)
	ccflags-y += -DILI_CONFIG_PANEL_NOTIFICATIONS
endif
ifneq ($(filter m y,$(CONFIG_ILITEK_RESUME_BY_DDI)),)
	ccflags-y += -DILI_CONFIG_RESUME_BY_DDI
endif
ifneq ($(filter m y,$(CONFIG_ILITEK_FW_PANEL)),)
	ccflags-y += -DILI_FW_PANEL
endif
ifneq ($(filter m y,$(CONFIG_ILITEK_ESD)),)
	ccflags-y += -DILI_CONFIG_ESD
endif
ifneq ($(filter m y,$(CONFIG_ILITEK_GESTURE)),)
	ccflags-y += -DILI_CONFIG_GESTURE
endif

ilitek_v3_mmi-y += ilitek_v3.o
ilitek_v3_mmi-y += ilitek_v3_spi.o
ilitek_v3_mmi-y += ilitek_v3_qcom.o
ilitek_v3_mmi-y += ilitek_v3_ic.o
ilitek_v3_mmi-y += ilitek_v3_touch.o
ilitek_v3_mmi-y += ilitek_v3_mp.o
ilitek_v3_mmi-y += ilitek_v3_hostdl.o
ilitek_v3_mmi-y += ilitek_v3_node.o

obj-$(CONFIG_TOUCHSCREEN_ILITEK_CHIPSET) += ilitek_v3_mmi.o
