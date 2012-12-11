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
#include <spartan/ipc.h>

#include "../mini_env.h"

static void entry(void)
{
	Genode::Native_ipc_call call;
	Genode::Native_thread_id thread_id = Spartan::thread_get_id();

	while(1)
	{
		call = Spartan::ipc_wait_for_call_timeout(0);
		Genode::printf("THREAD %lu:\treceived incomming call\n"
			       "\t   IMETHOD=%lu, ARG1=%lu, ARG2=%lu, ARG3=%lu"
			       ", ARG4=%lu, ARG5=%lu\n", thread_id,
			       IPC_GET_IMETHOD(call),
			       IPC_GET_ARG1(call), IPC_GET_ARG2(call),
			       IPC_GET_ARG3(call), IPC_GET_ARG4(call),
			       IPC_GET_ARG5(call));

		Spartan::ipc_answer_0(call.callid, 0, 0);
	}
}

/**
 * Main program, called by the _main() function
 */
extern "C" int main(void)
{
	enum { STACK_SIZE = 16384 };
	static char stack[STACK_SIZE];

	Spartan::thread_create((void*)&entry, &stack, STACK_SIZE, "waiting thread 1");
	Spartan::thread_create((void*)&entry, &stack, STACK_SIZE, "waiting thread 2");

	Spartan::ipc_call_async_fast(0, 5, 1, 2, 3, 4);
	Spartan::ipc_call_async_fast(0, 10, 6, 7, 8, 9);

	while(1);

	Spartan::exit(0);
}
