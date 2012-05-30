BASE_DIR = ../../..
SPARTAN_DIR = $(BASE_DIR)/contrib
SPARTAN_LIBC_DIR = $(SPARTAN_DIR)/uspace/lib/c

include $(SPARTAN_DIR)/Makefile.config

CXXFLAGS  = -I$(SPARTAN_LIBC_DIR)/include -nostdlib -nostdinc -fpermissive -fno-exceptions
#CXXFLAGS += -DCONFIG_MAX_THREAD_BITS=10
#CXXFLAGS += -DCONFIG_CPU_IA32_I586
#CXXFLAGS += -DCONFIG_KDB_CONS

LD_SCRIPT = genode.ld
#LD_SCRIPT = /home/kurbel/develop/genode/base-spartan/contrib/uspace/lib/c/arch/amd64/_link.ld

#LDFLAGS   = -Wl,-Ttext=0x01000000 -Wl,-T$(LD_SCRIPT)
#LDFLAGS   = -Wl,-Ttext=0000201000 -Wl,-T$(LD_SCRIPT)
#LDFLAGS   = -Wl,-Ttext=0000001100 -Wl,-T$(LD_SCRIPT)
#LDFLAGS   = -Wl,-Ttext=00000010e8 -Wl,-T$(LD_SCRIPT)
#LDFLAGS   = -Wl,-T$(LD_SCRIPT)
LDFLAGS   = -T$(LD_SCRIPT)

all: hello

hello: hello.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -static $(SPARTAN_LIBC_DIR)/arch/$(UARCH)/src/syscall.S crt0.s hello.cc -o $@

#
# Merge kernel and hello program into a single elf image
#
# We need to strip the elf image to make the image loadable by grub.
# Otherwise GRUB complaints:
#
# "Error 28: Selected item cannot fit into memory"
#
clean:
	rm -f hello
