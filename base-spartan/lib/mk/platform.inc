SPARTAN_SYS_INC += syscall.h \
                   sys/types.h \
                   abi/ddi/arg.h \
                   abi/syscall.h

all: $(addprefix $(BUILD_BASE_DIR)/include/,$(SPARTAN_SYS_INC))

$(BUILD_BASE_DIR)/include/abi/%: $(REP_DIR)/contrib/abi/include/%
	$(VERBOSE)mkdir -p $(dir $@)
	$(VERBOSE)cp $< $@

$(BUILD_BASE_DIR)/include/%: $(SPARTAN_LIBC_DIR)/include/%
	$(VERBOSE)mkdir -p $(dir $@)
	$(VERBOSE)cp $< $@
