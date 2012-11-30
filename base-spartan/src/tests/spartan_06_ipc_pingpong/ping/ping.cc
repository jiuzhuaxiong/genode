/*
 * \brief  Simple roottask replacement testing conntion handling and phone hanups
 * \author Tobias Börtitz
 * \date   2012-11-28
 */

/*
 * Copyright (C) 2008-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/printf.h>
#include <base/native_types.h>

/* Spartan includes */
#include <spartan/syscalls.h>
#include <spartan/ipc.h>

/* local includes */
#include "../../mini_env.h"

enum {
	PHONE_NAMESERV = 0,
	TASK_PONG = 4,
	THREAD_PONG = 7
};

int                 phone_nameserv, phone_pong;
Genode::addr_t      phonehash_nameserv;
Genode::Native_task task_id_nameserv;

bool register_with_nameserv()
{
	Genode::addr_t msg_callid = Spartan::ipc_connect_to_me(0);

	Genode::Native_ipc_call call = Spartan::ipc_wait_for_call_timeout(0);

	task_id_nameserv = call.in_task_id;
	phonehash_nameserv = call.in_phone_hash;
/*
	Genode::printf("pIng:register_with_nameserv: msg_callid=%lu, "
	               "call.callid=%lu & call.in_phone_hash=%lu\n",
	               msg_callid, call.callid, call.in_phone_hash);
*/
	if ((IPC_GET_RETVAL(call) == EOK) && (call.callid & IPC_CALLID_ANSWERED)
			&& (call.callid == msg_callid+1))
		return true;

	return false;
}

bool connect_to_pong()
{
	/*
	phone_pong = Spartan::ipc_connect_me_to(PHONE_NAMESERV, TASK_PONG,
	                                        THREAD_PONG,
	                                        Spartan::thread_get_id());

	return phone_pong>0 ? true : false;
	*/
	Genode::printf("pIng:\tconnect_to_pong() NOT IMPLEMENTED\n");
	return false;
}

int clone_pong_to_pong()
{
//	return Spartan::ipc_clone_connection(phone_pong, TASK_PONG, THREAD_PONG, phone_pong);
	Genode::printf("pIng:\tclone_pong_to_pong() NOT IMPLEMENTED\n");
	return -1;
}

/**
 * Main program, called by the _main() function
 */
extern "C" int main(void)
{
	Genode::Native_ipc_call   call;
	int                       ret;

	Genode::printf("pIng:\tping started\n");

	if(register_with_nameserv())
		/**
		 * Since it is an answer we are displaying here, there is no
		 *  incomming task_id.
		 */
		Genode::printf("pIng:\tregistered succefully with nameserv.\n"
		               "\t\tnameserv's task_d=%lu & phonehash=%lu\n",
		               task_id_nameserv, phonehash_nameserv);
	else
		Genode::printf("pIng:\tcould not register with nameserv.\n");

	if(connect_to_pong())
		Genode::printf("pIng:\tsuccessfully connected with pong\n");
	else
		Genode::printf("pIng:\tcould not connect to pong\n");

	ret = clone_pong_to_pong();
	Genode::printf("pIng:\tclone_pong_to_pong() returned %i\n", ret);

	call = Spartan::ipc_wait_for_call_timeout(0);
	if(call.in_phone_hash == phonehash_nameserv)
		Genode::printf("pIng:\treceived call with callid = %lu,\n"
		                "\t  in_task_id = %lu from known in_phone_hash = "
		                "%lu\n", call.callid, call.in_task_id, call.in_phone_hash);
	else
		Genode::printf("pIng:\treceived unknown call with callid = %lu,"
		               "\n\t  in_task_id = %lu, in_phone_hash = %lu\n", call.callid,
		               call.in_task_id, call.in_phone_hash);

	switch(IPC_GET_IMETHOD(call)) {
	case IPC_M_PHONE_HUNGUP:
		if(call.in_phone_hash == phonehash_nameserv)
			Genode::printf("pIng:\tnameserv hung up the connection.\n");
		else
			Genode::printf("pIng:\ttask %lu hung up the "
			               "connection.\n", call.in_task_id);
		break;
	default:
		Genode::printf("pIng:\tunhandled method %lu received", 
		               IPC_GET_IMETHOD(call));
	}

	while(1);

	Spartan::exit(0);
}

