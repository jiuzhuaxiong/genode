DIR := $(dir $(lastword $(MAKEFILE_LIST)))

CONTRIB_DIR := $(REP_DIR)/contrib/

CONFIG_MAKEFILE = $(DIR)/Makefile.config
CONFIG_HEADER = $(DIR)/config.h

all: config autotool
	$(MAKE) $(VERBOSE_DIR) -C $(CONTRIB_DIR)/kernel/ \
		VPATH=$(CONTRIB_DIR)/kernel
	$(MAKE) -C $(CONTRIB_DIR)/uspace/lib/c $(INCLUDE_ABI) $(INCLUDE_LIBARCH)
	cp $(CONTRIB_DIR)/kernel/kernel.bin bin/kernel

autotool:
	$(MAKE) $(VERBOSE_DIR) -C $(CONTRIB_DIR) \
		VPATH=$(DIR) \
		autotool

config:
	cp $(CONFIG_MAKEFILE) $(CONTRIB_DIR)
	cp $(CONFIG_HEADER) $(CONTRIB_DIR)

#	$(MAKE) $(VERBOSE_DIR) -f $(CONTRIB_DIR)/Makefile \
		-I $(DIR) \
		VPATH=$(CONTRIB_DIR) \
		autotool

#	$(MAKE) $(VERBOSE_DIR) -C ./ -f $(CONTRIB_DIR)/Makefile \
		VPATH=$(CONTRIB_DIR) \
		config
#	$(MAKE) $(VERBOSE_DIR) -f $(CONTRIB_DIR)/Makefile \
		VPATH=$(CONTRIB_DIR) \
		autotool

#	$(MAKE) $(VERBOSE_DIR)  $(CONTRIB_DIR)/kernel/Makefile \
		-I $(CONTRIB_DIR)/kernel \
		VPATH=$(CONTRIB_DIR)/kernel

#include $(REP_DIR)/contrib/Makefile

#TARGET			= kernel
#SPARTAN_SRC_DIR		= $(REP_DIR)/contrib/kernel
#SPARTAN_BUILD_DIR	= $(BUILD_BASE_DIR)/kernel
#REQUIRES 		+= spartan
#LIBS			= kernel

#SRC_CC   = hello.cc \
	$(REP_DIR)/contrib/uspace/lib/c/arch/$(UARCH)/src/syscall.S
#INC_DIR += $(REP_DIR)/contrib/uspace/lib/c/include

#figure out where to add the syscall.S from $(SPARTAN_LIBC_DIR)/arch/$(UARCH)/src/syscall.S !

