/*
 * \brief  Spartan syscalls
 * \author Tobias Börtitz
 * \date   2012-11-27
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__SPARTAN__IPC_H_
#define _INCLUDE__SPARTAN__IPC_H_

#include <base/stdint.h>
#include <base/native_types.h>

namespace Spartan
{
#include <sys/types.h>
#include <abi/ipc/ipc.h>
#include <abi/ipc/methods.h>


	/**
	 * Fast asynchronous call
	 */
	Genode::Native_ipc_callid ipc_call_async_fast(int phoneid,
	                                              Genode::addr_t imethod,
	                                              Genode::addr_t arg1,
	                                              Genode::addr_t arg2,
	                                              Genode::addr_t arg3,
	                                              Genode::addr_t arg4,
	                                              Genode::Native_ipc_call *retcall);
	/**
	 * Slower asynchronous call carrying more payload
	 */
	Genode::Native_ipc_callid ipc_call_async_slow(int phoneid,
	                                              Genode::addr_t imethod,
	                                              Genode::addr_t arg1,
	                                              Genode::addr_t arg2,
	                                              Genode::addr_t arg3,
	                                              Genode::addr_t arg4,
	                                              Genode::addr_t arg5,
	                                              Genode::Native_ipc_call *retcall);

	/**
	 * Look for incomming messages
	 */
	Genode::Native_ipc_callid ipc_wait_cycle(Genode::Native_ipc_call *call,
	                                          Genode::addr_t usec,
	                                          unsigned int flags);
	/**
	 * Wait for incomming calls with a timeout
	 */
	Genode::Native_ipc_callid
		ipc_wait_for_call_timeout(Genode::Native_ipc_call *call,
		                          Genode::addr_t usec);
	/**
	 * Wait for ever for incomming calls
	 */
	Genode::Native_ipc_callid
		ipc_trywait_for_call(Genode::Native_ipc_call *call);

	/**
	 * Fast forward a call
	 */
	int ipc_forward_fast(Genode::Native_ipc_callid callid, int phoneid,
	                     Genode::addr_t imethod, Genode::addr_t arg1,
	                     Genode::addr_t arg2, unsigned int mode);
	/**
	 * Slow forwarding a call
	 */
	int ipc_forward_slow(Genode::Native_ipc_callid callid, int phoneid,
	                     Genode::addr_t imethod, Genode::addr_t arg1,
	                     Genode::addr_t arg2, Genode::addr_t arg3,
	                     Genode::addr_t arg4, Genode::addr_t arg5,
	                     unsigned int mode);

	/**
	 * Answer a call fast
	 */
	Genode::addr_t ipc_answer_fast(Genode::Native_ipc_callid callid,
	                               Genode::addr_t retval,
	                               Genode::addr_t arg1, Genode::addr_t arg2,
	                               Genode::addr_t arg3, Genode::addr_t arg4);
	/**
	 * Answer a call slow
	 */
	Genode::addr_t ipc_answer_slow(Genode::Native_ipc_callid callid,
	                               Genode::addr_t retval,
	                               Genode::addr_t arg1, Genode::addr_t arg2,
	                               Genode::addr_t arg3, Genode::addr_t arg4,
	                               Genode::addr_t arg5);

	/**
	 * Request connection
	 */
	int ipc_connect_me_to(int phoneid, Genode::Native_thread_id dest_threadid,
	                      Genode::Native_thread_id my_threadid);

	/**
	 * Request callback connection
	 */
	int ipc_connect_to_me(int phoneid, Genode::Native_thread_id my_threadid,
	                      Genode::addr_t arg2, Genode::addr_t arg3, Genode::Native_task *task_id,
	                      Genode::addr_t *phonehash);

	/**
	 * Send a cloned phone
	 */
	int ipc_send_phone(int snd_phone, int clone_phone);

	/**
	 * Ask for a cloned phone
	 */
	int ipc_receive_phone(int snd_phone, Genode::Native_task dst_task_id,
	                      Genode::Native_thread_id dst_thread_id);

	/**
	 * Hangup a phone
	 */
	int ipc_hangup(int phoneid);
}

#endif /* _INCLUDE__SPARTAN__IPC_H_ */
