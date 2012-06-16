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

enum {
	PHONE_NAMESERV = 0,
};

int		phone_nameserv, phone_pong;
Genode::addr_t	phonehash_nameserv;
Genode::Native_task task_id_nameserv;

bool register_with_nameserv()
{
	phone_nameserv = Spartan::ipc_connect_to_me(PHONE_NAMESERV, 0, 0, 0,
			&task_id_nameserv, &phonehash_nameserv);
	return phonehash_nameserv ? true : false;
}

bool connect_to_pong()
{
	phone_pong = Spartan::ipc_connect_me_to(PHONE_NAMESERV, 4, 0, 0);

	return phone_pong>0 ? true : false;
}

/**
 * Main program, called by the _main() function
 */
extern "C" int main(void)
{
	Genode::Native_ipc_callid	callid;
	Genode::Native_ipc_call		call;

	Genode::printf("ping:\tping started\n");

	if(register_with_nameserv())
		Genode::printf("ping:\tregistered succefully with nameserv.\n");
	else
		Genode::printf("ping:\tcould not register with nameserv.\n");

	if(connect_to_pong())
		Genode::printf("ping:\tsuccessfully connected with pong\n");
	else
		Genode::printf("ping:\tcould not connect to pong\n");

	callid = Spartan::ipc_wait_for_call_timeout(&call, 0);
	if(call.in_phone_hash == phonehash_nameserv)
		Genode::printf("ping:\treceived call with callid = %lu,\n"
			"\t  in_task_id = %lu from known in_phone_hash = "
			"%lu\n", callid, call.in_task_id, call.in_phone_hash);
	else
		Genode::printf("ping:\treceived unknown call with callid = %lu,"
			"\n\t  in_task_id = %lu, in_phone_hash = %lu\n", callid,
			call.in_task_id, call.in_phone_hash);
	switch(IPC_GET_IMETHOD(call)) {
		case IPC_M_PHONE_HUNGUP:
			if(call.in_phone_hash == phonehash_nameserv)
				Genode::printf("ping:\tnameserv hung up the "
					"connection.\n");
			else
				Genode::printf("ping:\ttask %lu hung up the "
					"connection.\n", call.in_task_id);
			break;
		default:
			Genode::printf("ping:\tunhandled method %lu received", 
				IPC_GET_IMETHOD(call));
	}

	while(1);

	Spartan::exit(0);
}

