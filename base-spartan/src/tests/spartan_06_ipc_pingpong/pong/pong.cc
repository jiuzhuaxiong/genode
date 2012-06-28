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

int		phone_nameserv;
Genode::addr_t	phonehash_nameserv;
Genode::Native_task task_id_nameserv;

bool register_with_nameserv()
{
	phone_nameserv = Spartan::ipc_connect_to_me(0, Spartan::thread_get_id(), 0, 0,
		&task_id_nameserv, &phonehash_nameserv);
	return phonehash_nameserv ? true : false;
}


Genode::addr_t reject_connection(Genode::Native_ipc_callid callid)
{
	Genode::printf("pong: rejecting callid %lu\n", callid);
	return Spartan::ipc_answer_0(callid, -1);
}

Genode::addr_t accept_connection(Genode::Native_ipc_callid new_callid,
		Genode::addr_t new_phonehash, Genode::Native_task new_task_id,
		int new_phone)
{
	Genode::printf("pong:\taccepting incomming connection with "
		"task_id=%lu\n", new_task_id);
	return Spartan::ipc_answer_0(new_callid, 0);
}
/**
 * Main program, called by the _main() function
 */
extern "C" int main(void)
{
	Genode::Native_ipc_callid	callid;
	Genode::Native_ipc_call		call;
	Genode::Native_task		my_task = Spartan::task_get_id();
	Genode::Native_thread_id	my_threadid = Spartan::thread_get_id();
	Genode::addr_t			retval;

	Genode::printf("pong:\tpong started\n");

	if(register_with_nameserv())
		Genode::printf("pong:\tregistered succefully with nameserv.\n");
	else
		Genode::printf("pong:\tcould not register with nameserv.\n");

	while(1) {
	callid = Spartan::ipc_wait_for_call_timeout(&call, 0);
	Genode::printf("pong:\treceived incomming call\n"
			"\t   IMETHOD=%lu, ARG1=%lu, ARG2=%lu, ARG3=%lu,"
			" ARG4=%lu, ARG5=%lu\n", 
			IPC_GET_IMETHOD(call),
			IPC_GET_ARG1(call), IPC_GET_ARG2(call),
			IPC_GET_ARG3(call), IPC_GET_ARG4(call),
			IPC_GET_ARG5(call));
	switch(IPC_GET_IMETHOD(call)) {
		case IPC_M_PHONE_HUNGUP:
			if(call.in_phone_hash == phonehash_nameserv)
				Genode::printf("pong:\tnameserv hung up the "
					"connection.\n");
			else
				Genode::printf("pong:\ttask %lu hung up the "
					"connection.\n", call.in_task_id);
			break;
		case IPC_M_CONNECT_TO_ME:
			Genode::printf("pong:\treceived connection request with"
				" callid = %lu,\n\t   in_task_id = %lu, "
				"in_phone_hash = %lu\n", callid,
				call.in_task_id, call.in_phone_hash);
			retval = accept_connection(callid, call.in_phone_hash,
				call.in_task_id, IPC_GET_ARG5(call));
			if(retval == EOK) 
				Genode::printf("pong:\tconnection established "
					"to task %lu\n", call.in_task_id);
			else
				Genode::printf("pong:\tcould not establish "
					"connection. Errorcode %lu\n", retval);
			break;
		case IPC_M_CONNECT_ME_TO:
			Genode::printf("pong:\treceived connection request to "
				" task=%lu & thread=%lu with callid = %lu,\n\t   "
				"in_task_id = %lu, in_thread_id=%lu, "
				"in_phone_hash = %lu\n",
				IPC_GET_ARG1(call), IPC_GET_ARG2(call), callid,
				call.in_task_id, IPC_GET_ARG3(call), call.in_phone_hash);
			if(my_task == IPC_GET_ARG1(call) && my_threadid == IPC_GET_ARG2(call)) {
				retval = accept_connection(callid, 
					call.in_phone_hash, call.in_task_id,
					IPC_GET_ARG5(call));
				if(retval == EOK)
					Genode::printf("pong:\tconnection "
						"established to task %lu\n",
						call.in_task_id);
				else
					Genode::printf("pong:\tcould not "
						"establish connection. "
						"Errorcode %lu\n", retval);
				break;
			}
			else
				Genode::printf("pong:\treceived "
					"IPC_M_CONNECT_ME_TO to task %lu "
					"request but do not kow it\n",
					IPC_GET_ARG1(call));
			break;
		case IPC_M_CONNECTION_CLONE: {
			int phone_ping = IPC_GET_ARG1(call);
			Spartan::ipc_answer_0(callid, 0);
			Spartan::ipc_call_async_fast(phone_ping, 11, 1,2,3,4);
			break;
					     }
		default:
			retval = reject_connection(callid);
			Genode::printf("pong:\tunhandled method %lu received",
				IPC_GET_IMETHOD(call));
	}
	}

	Spartan::exit(0);
}

