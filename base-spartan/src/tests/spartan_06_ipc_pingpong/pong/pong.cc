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

#include "../../mini_env.h"

/**
 * Main program, called by the _main() function
 */
extern "C" int main(void)
{
	Genode::Native_ipc_callid 	callid = 0;
	Genode::Native_ipc_call 	call;
	Genode::addr_t			retval;

	Genode::printf("pong:\tpong started\n");

//	callid = Spartan::ipc_trywait_for_call(&call);
	callid = Spartan::ipc_wait_for_call_timeout(&call, 0);
	Genode::printf("pong:\treceived PING with callid = %lu,\n"
			"\t   in_task_id = %lu, in_phone_hash = %lu\n", callid,
			call.in_task_id, call.in_phone_hash);

	Genode::printf("pong:\tsending PONG\n");
	retval = Spartan::ipc_answer_0(callid, 0);
	Genode::printf("pong:\tPONG sent. syscall returned %lu\n", retval);

	while(1);

	Spartan::exit(0);
}

