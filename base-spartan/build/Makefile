-include etc/build.conf

PWD            := $(shell pwd)
BUILD_BASE_DIR := $(PWD)
INSTALL_DIR    := $(PWD)/bin

export REPOSITORIES     ?= $(BASE_DIR:%base=%base-linux) $(BASE_DIR)
export VERBOSE          ?= @
export VERBOSE_DIR      ?= --no-print-directory
export VERBOSE_MK       ?= @
export ECHO             ?= echo -e

#
# Convert user-defined directories to absolute directories
#
# The 'echo' shell command expands '~' characters to the home directory,
# 'realpath' converts relative path names to absolute.
#
REPOSITORIES := $(realpath $(shell echo $(REPOSITORIES)))
BASE_DIR     := $(realpath $(shell echo $(BASE_DIR)))

#
# Configure shell program before executing any shell commands. On Ubuntu the
# standard shell is dash, which breaks colored output via its built-in echo
# command.
#
export SHELL := $(shell which bash)

select_from_repositories = $(firstword $(foreach REP,$(REPOSITORIES),$(wildcard $(REP)/$(1))))

include $(BASE_DIR)/mk/global.mk

CONTRIB_DIR    := $(call select_from_repositories,contrib)

#
# Some compilers do not support the compiler arguments that we use with 'gcc'
# and, consequently, spit errors. Hence, we have to check if the compiler
# arguments are supported and drop them in the other case. We cache the result
# of the check in the CC_OPT_CHECKCC variable. The caching improves the build
# performance by 5 to 10 percent.
#
checkcc = $(shell if $(CUSTOM_CC) $(1) -o /dev/null -xc - <<< 'int main(void){return 0;}' &> /dev/null; then echo "$(1)" ; fi ;)

CC_OPT_CHECKCC  = $(call checkcc, -static)
CC_OPT_CHECKCC += $(call checkcc, -fno-stack-protector)

export CC_OPT_CHECKCC

export LIBGCC_INC_DIR = $(shell dirname `$(CUSTOM_CXX_LIB) -print-libgcc-file-name`)/include

#
# Find out about the target directories to build
#
DST_DIRS := $(filter-out all config kernel clean,$(MAKECMDGOALS))

ifeq ($(MAKECMDGOALS),)
DST_DIRS := *
endif

#
# Default rule: build all directories specified as make arguments
#
all $(DST_DIRS): config kernel
	@true

-include $(call select_from_repositories,etc/specs.conf)
-include $(BUILD_BASE_DIR)/etc/specs.conf
export SPEC_FILES := $(foreach SPEC,$(SPECS),$(call select_from_repositories,mk/spec-$(SPEC).mk))
include $(SPEC_FILES)
export SPECS

# Configure the kernel specific options
config:
	$(MAKE) $(VERBOSE_DIR) -C $(CONTRIB_DIR) config
	$(MAKE) $(VERBOSE_DIR) -C $(CONTRIB_DIR) autotool
#	$(MAKE) $(VERBOSE_DIR) -f $(CONTRIB_DIR)/Makefile \
		VPATH=$(CONTRIB_DIR) \
		config
#	$(MAKE) $(VERBOSE_DIR) -f $(CONTRIB_DIR)/Makefile \
		VPATH=$(CONTRIB_DIR) \
		autotool


.PHONY: kernel
kernel:
	mkdir -p spartan/arch/amd64/src/boot
	mkdir -p spartan/arch/amd64/src/debug
	mkdir -p spartan/arch/amd64/src/ddi
	mkdir -p spartan/arch/amd64/src/drivers
	mkdir -p spartan/arch/amd64/src/bios
	mkdir -p spartan/arch/amd64/src/mm
	mkdir -p spartan/arch/amd64/src/cpu
	mkdir -p spartan/arch/amd64/src/proc
	mkdir -p spartan/arch/amd64/src/smp
	mkdir -p spartan/genarch/src/acpi
	mkdir -p spartan/genarch/src/mm
	mkdir -p spartan/genarch/src/fb
	mkdir -p spartan/genarch/src/drivers/i8042
	mkdir -p spartan/genarch/src/drivers/ega
	mkdir -p spartan/genarch/src/kbrd
	mkdir -p spartan/genarch/src/multiboot
	mkdir -p spartan/generic/src/adt
	mkdir -p spartan/generic/src/console
	mkdir -p spartan/generic/src/cpu
	mkdir -p spartan/generic/src/ddi
	mkdir -p spartan/generic/src/debug
	mkdir -p spartan/generic/src/interrupt
	mkdir -p spartan/generic/src/main
	mkdir -p spartan/generic/src/proc
	mkdir -p spartan/generic/src/syscall
	mkdir -p spartan/generic/src/mm
	mkdir -p spartan/generic/src/lib
	mkdir -p spartan/generic/src/printf
	mkdir -p spartan/generic/src/time
	mkdir -p spartan/generic/src/preempt
	mkdir -p spartan/generic/src/synch
	mkdir -p spartan/generic/src/smp
	mkdir -p spartan/generic/src/ipc
	mkdir -p spartan/generic/src/security
	mkdir -p spartan/generic/src/sysinfo
	mkdir -p spartan/generic/src/udebug
	mkdir -p spartan/test/atomic
	mkdir -p spartan/test/btree
	mkdir -p spartan/test/avltree
	mkdir -p spartan/test/fault
	mkdir -p spartan/test/mm
	mkdir -p spartan/test/synch
	mkdir -p spartan/test/print
	mkdir -p spartan/test/thread
	mkdir -p spartan/test/debug
	$(MAKE) $(VERBOSE_DIR) -C spartan/ -f $(CONTRIB_DIR)/kernel/Makefile \
		-I $(CONTRIB_DIR)/kernel \
		VPATH=$(CONTRIB_DIR)/kernel

clean:
	rm -rf spartan
	$(MAKE) $(VERBOSE_DIR) -f $(CONTRIB_DIR)/Makefile clean

