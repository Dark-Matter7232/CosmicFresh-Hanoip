# Cirrus Logic pinctrl drivers
madera-pinctrl-objs		:= pinctrl-madera.o
ifeq ($(CONFIG_PINCTRL_CS47L35),y)
madera-pinctrl-objs		+= pinctrl-cs47l35.o
endif

obj-$(CONFIG_PINCTRL_MADERA)	+= madera-pinctrl.o
