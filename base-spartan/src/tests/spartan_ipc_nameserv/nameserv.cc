/*
 * \brief  Simple roottask replacement
 * \author Tobias Börtitz
 * \date   2021-11-28
 */

/*
 * Copyright (C) 2008-2012 Genode Labs GmbH
 *
 * Thir file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/printf.h>
#include <base/native_types.h>

/* Spartan includes */
#include <spartan/syscalls.h>
#include <spartan/ipc.h>
#include <spartan/errno.h>

#include "../mini_env.h"

enum {
	MAX_CONN_COUNT = 64,
};

Genode::addr_t            _phone_hash[MAX_CONN_COUNT];
Genode::addr_t            _callid[MAX_CONN_COUNT];
Genode::Native_thread_id  _thread_id[MAX_CONN_COUNT];
int                       _phone[MAX_CONN_COUNT];
int                       _call_counter = 0;

bool insert_new_connection(Genode::addr_t new_callid,
                           Genode::addr_t new_phonehash,
                           Genode::Native_thread_id new_thread_id, int new_phone)
{
	/** return false if new call would exceed maximum connection counts */
	if(_call_counter >= MAX_CONN_COUNT)
		return false;

	/** insert callid and phonehash into global vars */
	_callid[_call_counter] = new_callid;
	_phone_hash[_call_counter] = new_phonehash;
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

	/** if not found, return errorcode */
	return -1;
}


int connection_exists_thread_id(Genode::Native_thread_id thread_id)
{
	for(int i=0; i<_call_counter; i++)
		if(_thread_id[i] == thread_id) {
			Genode::printf("nameserv:connection_exists_thread_id: returning i=%i\n", i);
			return i;
		}

	return -1;
}

int delete_connection(Genode::addr_t phonehash)
{
	int pos, i;

	/** if found delete callid and phonehash from global vars */
	if((pos = connection_exists_phonehash(phonehash)) <= 0) 
		return pos;

	for(i=pos; (i+1)<_call_counter; i++) {
		_phone_hash[i] = _phone_hash[i+1];
		_callid[i] = _callid[i+1];
		_thread_id[i] = _thread_id[i+1];
		_phone[i] = _phone[i+1];
	}
	_call_counter--;
	return pos;
}

Genode::addr_t accept_connection(Genode::addr_t new_callid,
                                 Genode::addr_t new_phonehash,
                                 Genode::Native_thread_id new_thread_id,
                                 int new_phone)
{
	Genode::addr_t	retval;

	Genode::printf("nameserv:\taccepting incomming connection\n");
	retval = Spartan::ipc_answer_0(new_callid, new_thread_id, 0);

	if(retval == EOK) /* connection is established succefully */
		retval = insert_new_connection(new_callid, new_phonehash,
		                               new_thread_id, new_phone);

	return retval>=0 ? 0 : retval;
}

Genode::addr_t reject_connection(Genode::Native_ipc_call call)
{
	Genode::printf("nameserv: rejecting callid %lu\n", call.callid);

	return Spartan::ipc_answer_0(call.callid, IPC_GET_ARG4(call), E__IPC_CONNECTION_REJECTED);
}

/**
 * Main program, called by the _main() function
 */
extern "C" int main(void)
{
	Genode::Native_ipc_call   call;
	Genode::addr_t            retval;
	int pos;

	Genode::printf("nameserv:\tnameserv started\n");

	while(1) {
		call = Spartan::ipc_wait_for_call_timeout(0);
		Genode::printf("nameserv:\treceived incomming call\n"
		               "\t   IMETHOD=%lu, ARG1=%lu, ARG2=%lu, ARG3=%lu"
		               ", ARG4=%lu, ARG5=%lu\n", IPC_GET_IMETHOD(call),
		               IPC_GET_ARG1(call), IPC_GET_ARG2(call),
		               IPC_GET_ARG3(call), IPC_GET_ARG4(call),
		               IPC_GET_ARG5(call));
		switch(IPC_GET_IMETHOD(call)) {
		case IPC_M_CONNECT_TO_ME:
			Genode::printf("nameserv:\treceived callback "
			               "request with callid = %lu,\n\t   "
			               "in_task_id = %lu, in_phone_hash = %lu"
			               "\n", call.callid, call.in_task_id,
			               call.in_phone_hash);
			retval = accept_connection(call.callid,
					   call.in_phone_hash,
					   IPC_GET_ARG3(call),
					   IPC_GET_ARG1(call));
			if(retval == EOK) 
				Genode::printf("nameserv: callback "
				               "established to task %lu, "
				               "thread %lu\n", call.in_task_id,
				               IPC_GET_ARG1(call));
			else
				Genode::printf("nameserv:\tcould not "
				               "establish callback. "
				               "Errorcode %lu\n", retval);
			break;
		case IPC_M_CONNECT_ME_TO:
			Genode::printf("nameserv:\treceived connection "
			               "requestfrom thread=%lu to thread=%lu with "
			               "callid=%lu,\n\t  in_task_id=%lu, "
			               "in_phone_hash=%lu\n",
			               IPC_GET_ARG3(call), IPC_GET_ARG4(call), call.callid,
			               call.in_task_id, call.in_phone_hash);
//			if(int pos = connection_exists_thread_id(IPC_GET_ARG1(call)) >= 0) {
			pos = connection_exists_thread_id(IPC_GET_ARG4(call));
			if(pos >= 0) {
				Genode::printf("nameserv:\t forwarding call to thread=%lu via phone=%i where pos=%i\n", _thread_id[pos], _phone[pos], pos);
				/* TODO
				 * give more information to forwarding ipc call
				 */
				Spartan::ipc_forward_fast(call.callid,
				                          _phone[pos], _thread_id[pos], 0, 0,
				                          IPC_FF_NONE);
			}
			else {
					Genode::printf("nameserv:\tno thread with"
				               " id %lu known\n",
				               IPC_GET_ARG4(call));
					reject_connection(call);
			}
			break;
		default:
			retval = reject_connection(call);
			Genode::printf("nameserv:\tconnection from task %lu rejected "
			               "with returncode %lu\n", retval, call.in_task_id);
		}
	}

	Spartan::exit(0);
}

