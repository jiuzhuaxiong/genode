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

enum {
	MAX_CONN_COUNT = 64,
};

Genode::addr_t			_phone_hash[MAX_CONN_COUNT];
Genode::Native_ipc_callid	_callid[MAX_CONN_COUNT];
Genode::Native_task		_task_id[MAX_CONN_COUNT];
Genode::Native_thread_id	_thread_id[MAX_CONN_COUNT];
int				_phone[MAX_CONN_COUNT];
int				_call_counter = 0;

bool insert_new_connection(Genode::Native_ipc_callid new_callid,
		Genode::addr_t new_phonehash, Genode::Native_task new_task_id,
		Genode::Native_thread_id new_thread_id, int new_phone)
{
	/** return false if new call would exceed
	 * maximum connection counts
	 */
	if(_call_counter >= MAX_CONN_COUNT)
		return false;

	/** insert callid and phonehash into global vars */
	_callid[_call_counter] = new_callid;
	_phone_hash[_call_counter] = new_phonehash;
	_task_id[_call_counter] = new_task_id;
	_thread_id[_call_counter] = new_thread_id;
	_phone[_call_counter] = new_phone;
	_call_counter++;

	return true;
}

int connection_exists_phonehash(Genode::addr_t phonehash)
{
	/** return position of the phonehash if found */
	for(int i=0; i<_call_counter; i++)
		if(_phone_hash[i] == phonehash)
			return i;

	/** if not found, return error */
	return -1;
}

int connection_exists_task_thread_id(Genode::Native_task task_id,
		Genode::Native_thread_id thread_id)
{
	for(int i=0; i<_call_counter; i++)
		if((_task_id[i] == task_id) && (_thread_id[i] = thread_id))
			return i;

	return -1;
}

int delete_connection(Genode::addr_t phonehash)
{
	int pos;

	/** if found delete callid and phonehash from global vars */
	if((pos = connection_exists_phonehash(phonehash)) >= 0) {
			_callid[pos] = 0;
			_phone_hash[pos] = 0;
			_task_id[pos] = 0;
			_call_counter--;
	}
	return pos;
}

Genode::addr_t accept_connection(Genode::Native_ipc_callid new_callid,
		Genode::addr_t new_phonehash, Genode::Native_task new_task_id,
		Genode::Native_thread_id new_thread_id, int new_phone)
{
	Genode::addr_t	retval;

	Genode::printf("nameserv:\taccepting incomming connection\n");
	retval = Spartan::ipc_answer_0(new_callid, 0);

	if(retval == EOK) /* connection is established succefully */
		retval = insert_new_connection(new_callid, new_phonehash,
			new_task_id, new_thread_id, new_phone);

	return retval>=0 ? 0 : retval;
}

Genode::addr_t reject_connection(Genode::Native_ipc_callid callid)
{
	Genode::printf("nameserv: rejecting callid %lu\n", callid);
	return Spartan::ipc_answer_0(callid, -1);
}

/**
 * Main program, called by the _main() function
 */
extern "C" int main(void)
{
	Genode::Native_ipc_callid 	callid = 0;
	Genode::Native_ipc_call 	call;
	Genode::addr_t			retval;

	Genode::printf("nameserv:\tnameserv started\n");

	while(1) {
//		callid = Spartan::ipc_trywait_for_call(&call);
		callid = Spartan::ipc_wait_for_call_timeout(&call, 0);
		Genode::printf("nameserv:\treceived incomming call\n"
				"\t   IMETHOD=%lu, ARG1=%lu, ARG2=%lu, ARG3=%lu"
				", ARG4=%lu, ARG5=%lu\n", IPC_GET_IMETHOD(call),
				IPC_GET_ARG1(call), IPC_GET_ARG2(call),
				IPC_GET_ARG3(call), IPC_GET_ARG4(call),
				IPC_GET_ARG5(call));
		switch(IPC_GET_IMETHOD(call)) {
			case IPC_M_CONNECT_TO_ME:
				Genode::printf("nameserv:\treceived connection "
					"request with callid = %lu,\n\t   "
					"in_task_id = %lu, in_phone_hash = %lu"
					"\n", callid, call.in_task_id,
					call.in_phone_hash);
				retval = accept_connection(callid, 
					call.in_phone_hash, call.in_task_id, 
					IPC_GET_ARG1(call), IPC_GET_ARG5(call));
				if(retval == EOK) 
					Genode::printf("nameserv:\tconnection "
						"established to task %lu\n", 
						call.in_task_id);
				else
					Genode::printf("nameserv:\tcould not "
						"establish connection. "
						"Errorcode %lu\n", retval);
				break;
			case IPC_M_CONNECT_ME_TO:
				Genode::printf("nameserv:\treceived connection "
					"request to task=%lu & thread=%lu with "
					"callid=%lu,\n\t  in_task_id=%lu, "
					"in_thread_id=%lu, in_phone_hash=%lu\n",
					IPC_GET_ARG1(call), IPC_GET_ARG2(call), 
					callid, call.in_task_id,
					IPC_GET_ARG3(call),  call.in_phone_hash);
				if(int pos = connection_exists_task_thread_id(IPC_GET_ARG1(call), IPC_GET_ARG2(call)) >= 0) {
					Spartan::ipc_forward_fast(callid, 
						_phone[pos], IPC_GET_ARG1(call),
						IPC_GET_ARG2(call), 0, 
						IPC_FF_NONE);
					break;
				}
				else
					Genode::printf("nameserv:\tno task with"
						" id %lu known\n",
						IPC_GET_ARG1(call));
			default:
				retval = reject_connection(callid);
				Genode::printf("nameserv:\tconnection rejected "
					"with returncode %lu\n", retval);
		}
	}

	Spartan::exit(0);
}

