REQUIRES         = spartan x86_64
SRC_S           += syscall.S
COMMON_HEADER    = $(REP_DIR)/src/lib/platform/x86_64/common.h
SPARTAN_LIBC_DIR = $(REP_DIR)/contrib/uspace/lib/c
SPARTAN_SYS_INC  = libarch/common.h \
                   libarch/types.h

$(BUILD_BASE_DIR)/include/libarch/common.h: $(COMMON_HEADER)
	$(VERBOSE)mkdir -p $(dir $@)
	$(VERBOSE)cp $< $@

$(BUILD_BASE_DIR)/include/libarch/%: $(SPARTAN_LIBC_DIR)/arch/amd64/include/%
	$(VERBOSE)mkdir -p $(dir $@)
	$(VERBOSE)cp $< $@

include $(REP_DIR)/lib/mk/platform.inc

vpath syscall.S $(SPARTAN_LIBC_DIR)/arch/amd64/src
