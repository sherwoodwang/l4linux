
ifneq ($(CONFIG_L4_VPCI),)
  obj-y             += l4vpci.o
  obj-$(CONFIG_X86) += i386.o
else
 ifneq ($(CONFIG_X86),)
  include $(srctree)/arch/x86/pci/Makefile
  KBUILD_CPPFLAGS += -I$(srctree)/arch/x86/pci
 endif
endif

b		:= ../../x86/pci

common-y	:= $(b)/common.o
direct-y	:= $(b)/direct.o
fixup-y		:= $(b)/fixup.o
i386-y		:= $(b)/i386.o
irq-y		:= $(b)/irq.o
legacy-y	:= $(b)/legacy.o
mmconfig-y	:= $(b)/mmconfig.o
mmconfig_32-y	:= $(b)/mmconfig_32.o
mmconfig_64-y	:= $(b)/mmconfig_64.o
acpi-y		:= $(b)/acpi.o
init-y		:= $(b)/init.o
early-y		:= $(b)/early.o
amd_bus-y	:= $(b)/amd_bus.o
bus_numa-y	:= $(b)/bus_numa.o
intel_mid_pci-y := $(b)/intel_mid_pci.o
