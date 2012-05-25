BASE_DIR = ../../..
SPARTAN_DIR = $(BASE_DIR)/contrib

USPACE_PREFIX = $(SPARTAN_DIR)/uspace

CXXFLAGS = -I$(USPACE_PREFIX)/lib/c/include -O3 -imacros $(SPARTAN_DIR)/config.h -fexec-charset=UTF-8 -fwide-exec-charset=UTF-32LE -finput-charset=UTF-8 -ffreestanding -fno-builtin -nostdlib -nostdinc -Wall -Wextra -Wno-clobbered -Wno-unused-parameter -Werror-implicit-function-declaration -Wwrite-strings -pipe -g -D__LE__ -fno-omit-frame-pointer -fpermissive
#LINKER_SCRIPT = genode.ld

BINARY = hello
STATIC_NEEDED = y

SOURCES = \
	hello.cc

-include $(USPACE_PREFIX)/Makefile.common

#
clean:
	rm -f hello *.o *.map *.disasm
