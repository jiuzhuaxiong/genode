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

/* Genode includes */
#include <base/stdint.h>
#include <base/native_types.h>

/*Spartan includes */
#include <spartan/methods.h>

namespace Spartan
{
#include <sys/types.h>
#include <abi/ipc/ipc.h>

	/**
	 * Fast asynchronous call
	 */
	Genode::Native_ipc_callid ipc_call_async_fast(int phoneid,
	                                              Genode::addr_t imethod,
	                                              Genode::addr_t arg1,
	                                              Genode::addr_t arg2,
	                                              Genode::addr_t arg3,
	                                              Genode::addr_t arg4);
	/**
	 * Slower asynchronous call carrying more payload
	 */
	Genode::Native_ipc_callid ipc_call_async_slow(int phoneid,
	                                              Genode::addr_t imethod,
	                                              Genode::addr_t arg1,
	                                              Genode::addr_t arg2,
	                                              Genode::addr_t arg3,
	                                              Genode::addr_t arg4,
	                                              Genode::addr_t arg5);

	/**
	 * Look for incomming messages
	 */
	Genode::Native_ipc_call ipc_wait_cycle(Genode::addr_t usec,
	                                       unsigned int flags);
	/**
	 * Wait for incomming calls with a timeout
	 */
	Genode::Native_ipc_call	ipc_wait_for_call_timeout(Genode::addr_t usec);
	/**
	 * Wait for ever for incomming calls
	 */
	Genode::Native_ipc_call ipc_trywait_for_call();

	/**
	 * Fast forward a call
	 */
	Genode::addr_t ipc_forward_fast(Genode::addr_t callid, int phoneid,
	                                Genode::addr_t imethod, Genode::addr_t arg1,
	                                Genode::addr_t arg2, unsigned int mode);
	/**
	 * Slow forwarding a call
	 */
	Genode::addr_t ipc_forward_slow(Genode::addr_t callid, int phoneid,
	                                Genode::addr_t imethod, Genode::addr_t arg1,
	                                Genode::addr_t arg2, Genode::addr_t arg3,
	                                Genode::addr_t arg4, Genode::addr_t arg5,
	                                unsigned int mode);

	/**
	 * Answer a call fast
	 */
	Genode::addr_t ipc_answer_fast(Genode::addr_t callid,
	                               Genode::addr_t retval,
	                               Genode::addr_t arg1, Genode::addr_t arg2,
	                               Genode::addr_t arg3, Genode::addr_t arg4);
	/**
	 * Answer a call slow
	 */
	Genode::addr_t ipc_answer_slow(Genode::addr_t callid,
	                               Genode::addr_t retval,
	                               Genode::addr_t arg1, Genode::addr_t arg2,
	                               Genode::addr_t arg3, Genode::addr_t arg4,
	                               Genode::addr_t arg5);

	/**
	 * Answering wrapper functions
	 */
	inline Genode::addr_t ipc_answer_0(Genode::addr_t callid,
	                               Genode::Native_thread_id dest_threadid,
	                               Genode::addr_t retval) {
		return ipc_answer_fast(callid, retval, 0, 0, 0, dest_threadid);
	}

	inline Genode::addr_t ipc_answer_1(Genode::addr_t callid,
	                             Genode::Native_thread_id dest_threadid,
	                             Genode::addr_t retval, Genode::addr_t arg1) {
		return ipc_answer_fast(callid, retval, arg1, 0, 0,
		                       dest_threadid);
	}

	inline Genode::addr_t ipc_answer_2(Genode::addr_t callid,
	                            Genode::Native_thread_id dest_threadid,
	                            Genode::addr_t retval, Genode::addr_t arg1,
	                            Genode::addr_t arg2) {
		return ipc_answer_fast(callid, retval, arg1, arg2, 0,
		                       dest_threadid);
	}

	inline Genode::addr_t ipc_answer_3(Genode::addr_t callid,
	                            Genode::Native_thread_id dest_threadid,
	                            Genode::addr_t retval, Genode::addr_t arg1,
	                            Genode::addr_t arg2, Genode::addr_t arg3) {
		return ipc_answer_fast(callid, retval, arg1, arg2, arg3,
		                       dest_threadid);
	}

	inline Genode::addr_t ipc_answer_4(Genode::addr_t callid,
	                            Genode::Native_thread_id dest_threadid,
	                            Genode::addr_t retval, Genode::addr_t arg1,
	                            Genode::addr_t arg2, Genode::addr_t arg3,
	                            Genode::addr_t arg4) {
		return ipc_answer_slow(callid, retval, arg1, arg2, arg3,
		                       dest_threadid, arg4);
	}

	inline Genode::addr_t ipc_answer_5(Genode::addr_t callid,
	                            Genode::addr_t retval, Genode::addr_t arg1,
	                            Genode::addr_t arg2, Genode::addr_t arg3,
	                            Genode::addr_t arg4, Genode::addr_t arg5) {
		return ipc_answer_slow(callid, retval, arg1, arg2, arg3, arg4, arg5);
	}

	/**
	 * Request connection
	 */
	Genode::addr_t ipc_connect_me_to(int phoneid,
	                                 Genode::Native_thread_id dest_threadid,
	                                 Genode::Native_thread_id my_threadid);

	/**
	 * Request callback connection
	 */
	Genode::addr_t ipc_connect_to_me(int phoneid,
	                                 Genode::Native_thread_id dest_threadid,
	                                 Genode::Native_thread_id my_threadid);

	/**
	 * Send a cloned phone
	 */
	Genode::addr_t ipc_send_phone(int snd_phone, int clone_phone,
	                              Genode::Native_thread_id dst_threadid,
	                              Genode::Native_thread_id my_threadid);

	/**
	 * Ask for a cloned phone
	 */
	int ipc_receive_phone(int snd_phone, Genode::Native_task dst_task_id,
	                      Genode::Native_thread_id dst_thread_id);

	/**
	 * Hangup a phone
	 */
	int ipc_hangup(int phoneid, Genode::Native_thread_id dest_threadid,
	               Genode::Native_thread_id my_threadid);


	Genode::Native_ipc_callid ipc_data_write(int snd_phone,
	                                         const void* data,
	                                         size_t size,
	                                         Genode::Native_thread_id dst_threadid,
	                                         Genode::Native_thread_id my_threadid,
	                                         Genode::addr_t req_callid=0);
	Genode::addr_t ipc_data_accept(Genode::addr_t callid, void* data,
	                               size_t size,
	                               Genode::Native_thread_id snd_thread_id);
}

#endif /* _INCLUDE__SPARTAN__IPC_H_ */
