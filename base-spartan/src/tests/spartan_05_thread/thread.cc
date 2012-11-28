/*
 * \brief  Simple roottask replacement for OKL4 that just prints some text
 * \author Norman Feske
 * \date   2008-09-01
 */

/*
 * Copyright (C) 2008-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <base/printf.h>
#include <spartan/syscalls.h>

#include "../mini_env.h"

static void entry(void)
{
	while(1) {
		Genode::printf(" 1");
	}
}
/**
 * Main program, called by the _main() function
 */
extern "C" int main(void)
{
	enum { STACK_SIZE = 16384 };
	static char stack[STACK_SIZE];
	Genode::printf("This is Genode's printf\n");

	Spartan::thread_create((void*)&entry, &stack, STACK_SIZE, "thread");
	while(1)
		Genode::printf(" 0");

	Spartan::exit(0);
}
