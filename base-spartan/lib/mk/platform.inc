SRC_CC += syscalls.cc \
          futex.cc \
          as.cc \
          ipc.cc \
          klog.cc
>>>>>>> 623d4fb... Adds klog syscalls printing stuff onto SPARTAN's kernel console

SPARTAN_SYS_INC += syscall.h \
                   sys/types.h \
                   abi/ddi/arg.h \
                   abi/proc/uarg.h \
                   abi/ipc/ipc.h \
                   abi/ipc/methods.h \
                   abi/errno.h \
                   abi/synch.h \
                   abi/syscall.h \
                   abi/mm/as.h

all: $(addprefix $(BUILD_BASE_DIR)/include/,$(SPARTAN_SYS_INC))

$(BUILD_BASE_DIR)/include/abi/%: $(REP_DIR)/contrib/abi/include/%
	$(VERBOSE)mkdir -p $(dir $@)
	$(VERBOSE)cp $< $@

$(BUILD_BASE_DIR)/include/%: $(SPARTAN_LIBC_DIR)/include/%
	$(VERBOSE)mkdir -p $(dir $@)
	$(VERBOSE)cp $< $@

vpath syscalls.cc $(REP_DIR)/src/lib/platform
vpath futex.cc $(REP_DIR)/src/lib/platform
vpath as.cc $(REP_DIR)/src/lib/platform
vpath ipc.cc $(REP_DIR)/src/lib/platform
vpath klog.cc $(REP_DIR)/src/lib/platform
