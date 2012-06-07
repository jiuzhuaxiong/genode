TARGET      = kernel
CONTRIB_DIR = $(REP_DIR)/contrib/
KBUILD_DIR  = $(BUILD_BASE_DIR)/kernel
SPARTAN     = $(KBUILD_DIR)/kernel/kernel.bin

$(TARGET): $(SPARTAN)

$(SPARTAN): $(KBUILD_DIR)/Makefile.common
	$(MAKE) $(VERBOSE_DIR) -C $(CONTRIB_DIR)/kernel O=$(KBUILD_DIR)
	$(VERBOSE)ln -sf $@ $(BUILD_BASE_DIR)/bin/$(TARGET)

$(KBUILD_DIR)/Makefile.common: $(KBUILD_DIR)/config.h $(KBUILD_DIR)/Makefile.config
	$(MAKE) -C $(CONTRIB_DIR) O=$(KBUILD_DIR) autotool

$(KBUILD_DIR)/%: $(PRG_DIR)/%
	$(VERBOSE)mkdir -p $(KBUILD_DIR)
	$(VERBOSE)cp $< $@

clean cleanall:
	$(VERBOSE)rm -rf $(KBUILD_DIR) bin/kernel
