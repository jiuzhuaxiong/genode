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
	Genode::Native_task task_id = Spartan::task_get_id();
	int		myPhone;
	Genode::addr_t	phonehash;
	Genode::Native_ipc_callid	callid;
	Genode::Native_ipc_call		call;

	Genode::printf("ping:\tping started\n");

	Genode::printf("ping:\tsending PING\n");
	myPhone = Spartan::ipc_connect_to_me(0, 0, 0, 0, &task_id, &phonehash);
	Genode::printf("ping:\tPING sent. received phoneid = %i, taskid = %lu, phonehash = %lu\n", myPhone, task_id, phonehash);

	callid = Spartan::ipc_wait_for_call_timeout(&call, 0);
	Genode::printf("ping:\treceived PONG with callid = %lu,\n"
		"\t   in_task_id = %lu, in_phone_hash = %lu\n", callid,-
		call.in_task_id, call.in_phone_hash);

	while(1);

	Spartan::exit(0);
}

