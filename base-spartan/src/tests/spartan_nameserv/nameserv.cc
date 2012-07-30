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
#include <base/ipc_call.h>
#include <spartan/syscalls.h>

#include "../mini_env.h"

enum {
	MAX_CONN_COUNT = 64,

	IPC_CONNECTION_REQUEST = 30,
};

Genode::addr_t			_phone_hash[MAX_CONN_COUNT];
Genode::Native_ipc_callid	_callid[MAX_CONN_COUNT];
Genode::Native_thread_id	_thread_id[MAX_CONN_COUNT];
int				_phone[MAX_CONN_COUNT];
int				_call_counter = 0;

bool insert_new_connection(Genode::Native_ipc_callid new_callid,
		Genode::addr_t new_phonehash,
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

int connection_exists_thread_id(Genode::Native_thread_id thread_id)
{
	for(int i=0; i<_call_counter; i++)
		if(_thread_id[i] == thread_id)
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
			_call_counter--;
	}
	return pos;
}

/* implemente the connection cloning protocoll */
int answer_connection_request(Genode::Ipc_call call)
{
	int dst_pos, retval, snd_phone;
	/* return error if method of call is wrong */
	if(call.call_method() != IPC_CONNECTION_REQUEST)
		return -1;

	/* look up requested destination */
	dst_pos = connection_exists_thread_id(call.dest_thread_id());
	if(dst_pos >= 0) {
		/* we own a phone tp the destination */
		retval = Spartan::ipc_answer_0(call.callid(), 0);
		if(retval != 0)
			return -1;

		int snd_pos = connection_exists_thread_id(call.snd_thread_id());
		if (snd_pos < 0) {
		/* we own no phone to the sender */
			Genode::Native_ipc_callid	callid;
			Genode::Native_ipc_call		call;

			callid = Spartan::ipc_wait_for_call_timeout(&call, 0);
			if(IPC_GET_IMETHOD(call) != IPC_M_CONNECT_TO_ME)
				return -1;
			retval = Spartan::ipc_answer_0(callid, 0);
			snd_phone = IPC_GET_ARG5(call);
		}
		else
			snd_phone = _phone[snd_pos];
	}
	else
	{
		/* the requested destination is not known */
		retval = Spartan::ipc_answer_0(call.callid(), -1);
		return 0;
	}
	if(retval != 0)
		/* some error occured */
		return retval;

	return Spartan::ipc_clone_connection(snd_phone, _thread_id[dst_pos],
			1, 1, _phone[dst_pos]);
}

Genode::addr_t accept_connection(Genode::Native_ipc_callid new_callid,
		Genode::addr_t new_phonehash, 
		Genode::Native_thread_id new_thread_id, int new_phone)
{
	Genode::addr_t	retval;

	Genode::printf("nameserv:\taccepting incomming connection\n");
	retval = Spartan::ipc_answer_0(new_callid, 0);

	if(retval == EOK) /* connection is established succefully */
		retval = insert_new_connection(new_callid, new_phonehash,
			new_thread_id, new_phone);

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
					call.in_phone_hash, IPC_GET_ARG1(call),
					IPC_GET_ARG5(call));
				if(retval == EOK) 
					Genode::printf("nameserv:\tconnection "
						"established to task %lu, thread %lu\n", 
						call.in_task_id, IPC_GET_ARG1(call));
				else
					Genode::printf("nameserv:\tcould not "
						"establish connection. "
						"Errorcode %lu\n", retval);
				break;
			case IPC_M_CONNECT_ME_TO:
				Genode::printf("nameserv:\treceived connection "
					"request to thread=%lu with "
					"callid=%lu,\n\t  in_task_id=%lu, "
					"in_thread_id=%lu, in_phone_hash=%lu\n",
					IPC_GET_ARG1(call), callid, 
					call.in_task_id, IPC_GET_ARG2(call),
					call.in_phone_hash);
				if(int pos = connection_exists_thread_id(IPC_GET_ARG1(call)) >= 0) {
					Spartan::ipc_forward_fast(callid, 
						_phone[pos], 0, 0, 0,
						IPC_FF_ROUTE_FROM_ME);
				}
				else
					Genode::printf("nameserv:\tno thread with"
						" id %lu known\n",
						IPC_GET_ARG1(call));
				break;
			case 30: /* IPC_CONNECTION_REQUEST */
				answer_connection_request(Genode::Ipc_call(callid, call));
				break;
			default:
				retval = reject_connection(callid);
				Genode::printf("nameserv:\tconnection from task %lu rejected "
					"with returncode %lu\n", retval, call.in_task_id);
		}
	}

	Spartan::exit(0);
}

