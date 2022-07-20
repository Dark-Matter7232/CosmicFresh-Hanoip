# Undo compiler optimizations to workaround buggy code
ccflags-remove-y := -Os -O2 -O3 $(CC_FLAGS_LTO)

obj-$(CONFIG_QPNP_ADAPTIVE_CHARGE) += qpnp_adaptive_charge.o
