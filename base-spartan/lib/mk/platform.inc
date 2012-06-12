SRC_CC += syscalls.cc

SPARTAN_SYS_INC += syscall.h \
                   sys/types.h \
                   abi/ddi/arg.h \
		   abi/proc/uarg.h \
		   abi/ipc/ipc.h \
		   abi/ipc/methods.h \
		   abi/errno.h \
		   abi/synch.h \
                   abi/syscall.h

all: $(addprefix $(BUILD_BASE_DIR)/include/,$(SPARTAN_SYS_INC))

$(BUILD_BASE_DIR)/include/abi/%: $(REP_DIR)/contrib/abi/include/%
	$(VERBOSE)mkdir -p $(dir $@)
	$(VERBOSE)cp $< $@

$(BUILD_BASE_DIR)/include/%: $(SPARTAN_LIBC_DIR)/include/%
	$(VERBOSE)mkdir -p $(dir $@)
	$(VERBOSE)cp $< $@

vpath syscalls.cc $(REP_DIR)/src/lib/platform
