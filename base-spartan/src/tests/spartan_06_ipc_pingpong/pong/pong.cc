/*
 * \brief  Simple roottask replacement testing conecting and phone hangups
 * \author Tobias Börtitz
 * \date   2021-11-28
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

int                 phone_ping;
Genode::addr_t      phonehash_nameserv;
Genode::Native_task task_id_nameserv;

bool register_with_nameserv()
{
	Genode::addr_t msg_callid = Spartan::ipc_connect_to_me(0);

	Genode::Native_ipc_call call = Spartan::ipc_wait_for_call_timeout(0);

/*
	Genode::printf("pOng:register_with_nameserv:\treceived: callid=%lu, "
	               "in_task_id=%lu, in_phonehash=%lu\t IMETHOD=%lu, "
	               "ARG1=%lu, ARG2=%lu, ARG3=%lu, ARG4=%lu, ARG5=%lu\n",
	               call.callid, call.in_task_id, call.in_phone_hash,
	               IPC_GET_IMETHOD(call), IPC_GET_ARG1(call), IPC_GET_ARG2(call),
	               IPC_GET_ARG3(call), IPC_GET_ARG4(call), IPC_GET_ARG5(call));
	if(call.callid & IPC_CALLID_ANSWERED)
		Genode::printf("POW!\n");
*/
	task_id_nameserv = call.in_task_id;
	phonehash_nameserv = call.in_phone_hash;
/*
	Genode::printf("pOng:register_with_nameserv: msg_callid=%lu, "
	               "call.callid=%lu & call.in_phone_hash=%lu\n",
	               msg_callid, call.callid, call.in_phone_hash);
*/
	if ((IPC_GET_RETVAL(call) == EOK) && (call.callid & IPC_CALLID_ANSWERED)
			&& (call.callid == msg_callid+1))
		return true;

	return false;
}


Genode::addr_t reject_connection(Genode::addr_t callid)
{
	Genode::printf("pOng: rejecting callid %lu\n", callid);
	
	return Spartan::ipc_answer_0(callid, -1);
}

Genode::addr_t accept_connection(Genode::addr_t new_callid,
                                 Genode::addr_t new_phonehash,
                                 Genode::Native_task new_task_id,
                                 int new_phone)
{
	Genode::printf("pOng:\taccepting incomming connection with "
	               "task_id=%lu\n", new_task_id);

	return Spartan::ipc_answer_0(new_callid, 0);
}

/**
 *  * Main program, called by the _main() function
 *   */
extern "C" int main(void)
{
	Genode::Native_ipc_call   call;
	Genode::Native_task       my_task = Spartan::task_get_id();
	Genode::Native_thread_id  my_threadid = Spartan::thread_get_id();
	Genode::addr_t            retval;

	Genode::printf("pOng:\tpong started\n");

	if(register_with_nameserv())
		/**
		 * Since it is an answer we are displaying here, there is no
		 *  incomming task_id.
		 */
		Genode::printf("pOng:\tregistered succefully with nameserv.\n"
		               "\t\tnameserv's task_d=%lu & phonehash=%lu\n",
		               task_id_nameserv, phonehash_nameserv);
	else
		Genode::printf("pOng:\tcould not register with nameserv.\n");

	while(1) {
		call = Spartan::ipc_wait_for_call_timeout(0);
		Genode::printf("pOng:\treceived incomming call\n"
		               "\t   IMETHOD=%lu, ARG1=%lu, ARG2=%lu, ARG3=%lu,"
		               " ARG4=%lu, ARG5=%lu\n", IPC_GET_IMETHOD(call),
		               IPC_GET_ARG1(call), IPC_GET_ARG2(call),
		               IPC_GET_ARG3(call), IPC_GET_ARG4(call),
		               IPC_GET_ARG5(call));
		switch(IPC_GET_IMETHOD(call)) {
		case IPC_M_PHONE_HUNGUP:
			if(call.in_phone_hash == phonehash_nameserv)
				Genode::printf("pOng:\tnameserv hung up the "
				               "connection.\n");
			else
				Genode::printf("pOng:\ttask %lu hung up the "
				               "connection.\n", call.in_task_id);
			break;
		case IPC_M_CONNECT_TO_ME:
			Genode::printf("pOng:\treceived callback request with"
			               " callid = %lu,\n\t in_task_id = %lu, "
			               "in_phone_hash = %lu\n", call.callid,
			               call.in_task_id, call.in_phone_hash);
			retval = accept_connection(call.callid, call.in_phone_hash,
			                           call.in_task_id,
			                           IPC_GET_ARG5(call));
			if(retval == EOK) 
				Genode::printf("pOng:\tcallback established "
				               "to task %lu\n", call.in_task_id);
			else
				Genode::printf("pOng:\tcould not establish "
				               "callback. Errorcode %lu\n", retval);
			break;
		case IPC_M_CONNECT_ME_TO:
			Genode::printf("pOng:\treceived connection request to "
			               " task=%lu & thread=%lu with callid = %lu,\n\t   "
			               "in_task_id = %lu, in_thread_id=%lu, "
			               "in_phone_hash = %lu\n",
			               IPC_GET_ARG1(call), IPC_GET_ARG2(call),
			               call.callid, call.in_task_id,
			               IPC_GET_ARG3(call), call.in_phone_hash);
			if(my_task == IPC_GET_ARG1(call) && my_threadid == IPC_GET_ARG2(call)) {
				retval = accept_connection(call.callid,
				                           call.in_phone_hash,
				                           call.in_task_id,
				                           IPC_GET_ARG5(call));
				if(retval == EOK)
					Genode::printf("pOng:\tconnection "
					               "established to task %lu\n",
					               call.in_task_id);
				else
					Genode::printf("pOng:\tcould not "
					               "establish connection. "
					               "Errorcode %lu\n", retval);
				break;
			}
			else
				Genode::printf("pOng:\treceived "
				               "IPC_M_CONNECT_ME_TO to task %lu "
				               "request but do not kow it\n",
				               IPC_GET_ARG1(call));
			break;
		case IPC_M_CONNECTION_CLONE:
			phone_ping = IPC_GET_ARG1(call);
			Spartan::ipc_answer_0(call.callid, 0);
			Spartan::ipc_call_async_fast(phone_ping, 11, 1,2,3,4);
			break;
		default:
			retval = reject_connection(call.callid);
			Genode::printf("pOng:\tunhandled method %lu received",
			               IPC_GET_IMETHOD(call));
		}
	}

	Spartan::exit(0);
}
