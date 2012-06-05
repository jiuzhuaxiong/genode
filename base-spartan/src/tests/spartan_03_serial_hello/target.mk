include $(REP_DIR)/contrib/Makefile.config

TARGET   = hello
REQUIRES = spartan
SRC_CC   = $(REP_DIR)/contrib/uspace/lib/c/arch/$(UARCH)/src/syscall.S \
	   hello.cc
INC_DIR += $(REP_DIR)/contrib/uspace/lib/c/include

#figure out where to add the syscall.S from $(SPARTAN_LIBC_DIR)/arch/$(UARCH)/src/syscall.S !
