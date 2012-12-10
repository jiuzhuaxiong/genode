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
	THREAD_NAMESERV = 5,
	THREAD_PONG = 6
};

int                 phone_nameserv, phone_pong;
Genode::addr_t      phonehash_nameserv;
Genode::Native_task task_id_nameserv;

bool register_with_nameserv()
{
	Genode::addr_t msg_callid = Spartan::ipc_connect_to_me(PHONE_NAMESERV,
	                                                       THREAD_NAMESERV, Spartan::thread_get_id());

	Genode::Native_ipc_call call = Spartan::ipc_wait_for_call_timeout(0);

	task_id_nameserv = call.in_task_id;
	phonehash_nameserv = call.in_phone_hash;
/*
	Genode::printf("pIng:register_with_nameserv: msg_callid=%lu, "
	               "call.callid=%lu & call.in_phone_hash=%lu\n",
	               msg_callid, call.callid, call.in_phone_hash);
*/
	if ((IPC_GET_RETVAL(call) == EOK) && (call.callid & IPC_CALLID_ANSWERED)
			&& (call.callid == (msg_callid | IPC_CALLID_ANSWERED)))
		return true;

	return false;
}

bool connect_to_pong()
{
	Genode::addr_t msg_callid;
	msg_callid = Spartan::ipc_connect_me_to(PHONE_NAMESERV, THREAD_PONG,
	                                        Spartan::thread_get_id());

	Genode::Native_ipc_call call = Spartan::ipc_wait_for_call_timeout(0);
	phone_pong = IPC_GET_ARG5(call);
	Genode::printf("pIng:connect_to_pong: phone_pong = %lu\n", phone_pong);

	if ((IPC_GET_RETVAL(call) == EOK) && (call.callid & IPC_CALLID_ANSWERED)
			&& (call.callid == (msg_callid | IPC_CALLID_ANSWERED)))
		return true;
	return false;
}

bool clone_pong_to_pong()
{
	Genode::addr_t msg_callid;
	msg_callid = Spartan::ipc_send_phone(phone_pong, phone_pong, THREAD_PONG);

	Genode::Native_ipc_call call = Spartan::ipc_wait_for_call_timeout(0);

	if ((IPC_GET_RETVAL(call) == EOK) && (call.callid & IPC_CALLID_ANSWERED)
			&& (call.callid == (msg_callid | IPC_CALLID_ANSWERED)))
		return true;
	return false;
}

bool hangup_pong()
{
	Genode::addr_t msg_callid;
	msg_callid = Spartan::ipc_hangup(phone_pong, THREAD_PONG, Spartan::thread_get_id());

	Genode::Native_ipc_call call = Spartan::ipc_wait_for_call_timeout(0);

	if ((IPC_GET_RETVAL(call) == EOK) && (call.callid & IPC_CALLID_ANSWERED)
			&& (call.callid == (msg_callid | IPC_CALLID_ANSWERED)))
		return true;
	return false;
}

/**
 * Main program, called by the _main() function
 */
extern "C" int main(void)
{
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

	if(clone_pong_to_pong())
		Genode::printf("pIng:\tsuccessfully cloned to pong\n");
	else
		Genode::printf("pIng:\tcould not clone phone to pong\n");

	hangup_pong();


	while(1);

	Spartan::exit(0);
}

