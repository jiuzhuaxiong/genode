#
# \brief  Checkout HelenOS from official repository
# \author Tobias Börtitz
# \date   2012-05-02
#

VERBOSE    ?= @
ECHO        = @echo
BZR_URI     = bzr://bzr.helenos.org/mainline
BZR_REV     = 1530
#BZR_REV     = 1486
CONTRIB_DIR = contrib
PATCHES     = $(shell find patches -name *.patch)

#
# Print help information by default
#
help::

# realpath is there to follow symlink; if contrib dir does not exists yet,
# create new directory
REAL_DIR := $(realpath)
REAL_CONTRIB_DIR := $(realpath $(CONTRIB_DIR))
ifeq ($(REAL_CONTRIB_DIR),)
REAL_CONTRIB_DIR := $(CONTRIB_DIR)
endif

prepare: $(REAL_CONTRIB_DIR)/.bzr update_contrib_subdirs apply_patches

help::
	$(ECHO)
	$(ECHO) "Check out upstream source code of HelenOS"
	$(ECHO)
	$(ECHO) "The source code will be located at the '$(CONTRIB_DIR)/' directory."
	$(ECHO)
	$(ECHO) "--- available commands ---"
	$(ECHO) "prepare - checkout upstream source codes"
	$(ECHO) "clean   - remove upstream source codes"
	$(ECHO)

$(CONTRIB_DIR):
	$(VERBOSE)mkdir -p $@

# for resolving the dependencies of 'update_contrib_subdirs'
$(REAL_CONTRIB_DIR)/%:
	$(ECHO) "Checking out revision $(BZR_REV)"
#	$(VERBOSE)bzr checkout $(BZR_URI) $(REAL_CONTRIB_DIR)
	$(VERBOSE)bzr checkout -r $(BZR_REV) $(BZR_URI) $(REAL_CONTRIB_DIR)

# used phony to always update the SVN on 'make prepare'
# (before updating, we need to revert our custom patches)
update_contrib_subdirs:
	$(ECHO) "updating $(REAL_CONTRIB_DIR) to revision $(BZR_REV)"
	$(VERBOSE)bzr revert $(REAL_CONTRIB_DIR)

apply_patches:
	$(ECHO) "applying patches to '$(REAL_CONTRIB_DIR)/'"
	$(VERBOSE)for i in $(PATCHES); do \
	            patch -N -d $(REAL_CONTRIB_DIR) -p0 < $$i; done

# if $(CONTRIB_DIR) is a symlink, leave $(REAL_CONTRIB_DIR) alone
clean::
	$(VERBOSE)rm -rf $(CONTRIB_DIR)

.NOTPARALLEL:
