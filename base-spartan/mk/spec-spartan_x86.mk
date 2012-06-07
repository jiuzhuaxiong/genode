#
# Specifics for SPARTAN on amd64
#

SPECS += x86_64 spartan
#SPECS += pci ps2 vesa

#
# Linker options specific for x86
#
#LD_TEXT_ADDR ?= 0x00200000

#
# Also include less-specific configuration last
#
include $(call select_from_repositories,mk/spec-x86_64.mk)
include $(call select_from_repositories,mk/spec-spartan.mk)
