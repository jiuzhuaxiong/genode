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

CONTRIB_DIR    := $(call select_from_repositories,contrib)

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

# Configure the kernel specific options
config:
#	$(MAKE) $(VERBOSE_DIR) -C $(CONTRIB_DIR) config
#	$(MAKE) $(VERBOSE_DIR) -C $(CONTRIB_DIR) autotool
	$(MAKE) $(VERBOSE_DIR) -f $(CONTRIB_DIR)/Makefile \
		VPATH=$(CONTRIB_DIR) \
		config
	$(MAKE) $(VERBOSE_DIR) -f $(CONTRIB_DIR)/Makefile \
		VPATH=$(CONTRIB_DIR) \
		autotool


.PHONY: kernel
kernel:
	$(MAKE) $(VERBOSE_DIR) -f $(CONTRIB_DIR)/kernel/Makefile \
		-I $(CONTRIB_DIR)/kernel \
		VPATH=$(CONTRIB_DIR)/kernel

clean:
	$(MAKE) $(VERBOSE_DIR) -f $(CONTRIB_DIR)/Makefile clean

