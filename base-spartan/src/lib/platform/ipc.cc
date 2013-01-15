/*
 * \brief  Spartan-specific syscall implemetation
 * \author Tobias Börtitz
 * \date   2012-1-27
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <util/string.h>
#include <base/printf.h>

/* Spartan includes */
#include <spartan/syscalls.h>
#include <spartan/ipc.h>

using namespace Genode;

extern "C" {
#include <abi/ipc/ipc.h>
#include <abi/ipc/methods.h>
#include <abi/synch.h>
}


using namespace Spartan;


/**************************************
 *  Send requests & wrapper functions *
 **************************************/

Native_ipc_callid Spartan::ipc_call_async_fast(int phoneid, addr_t imethod,
                                               addr_t arg1, addr_t arg2,
                                               addr_t arg3, addr_t arg4)
{
	return __SYSCALL6(SYS_IPC_CALL_ASYNC_FAST, phoneid, imethod,
	                  arg1, arg2, arg3, arg4);
}


Native_ipc_callid Spartan::ipc_call_async_slow(int phoneid, addr_t imethod,
                                               addr_t arg1, addr_t arg2,
                                               addr_t arg3, addr_t arg4,
                                               addr_t arg5)
{
	Native_ipc_call   call;

	IPC_SET_IMETHOD(call, imethod);
	IPC_SET_ARG1(call, arg1);
	IPC_SET_ARG2(call, arg2);
	IPC_SET_ARG3(call, arg3);
	IPC_SET_ARG4(call, arg4);
	IPC_SET_ARG5(call, arg5);

	return __SYSCALL2(SYS_IPC_CALL_ASYNC_SLOW, phoneid,
	                  (sysarg_t) &call);
}



/*******************************
 * Wait for incomming requests *
 *******************************/

Native_ipc_call Spartan::ipc_wait_cycle(addr_t usec, unsigned int flags)
{
	Native_ipc_call call;

	call.callid = __SYSCALL3(SYS_IPC_WAIT, (addr_t) &call, usec, flags);
/* DEBUG
	Genode::printf("Spartan::ipc_wait_cycle:\treceived: callid=%lu, "
	               "in_task_id=%lu, in_phonehash=%lu\t IMETHOD=%lu, "
	               "ARG1=%lu, ARG2=%lu, ARG3=%lu, ARG4=%lu, ARG5=%lu\n",
	               call.callid, call.in_task_id, call.in_phone_hash,
	               IPC_GET_IMETHOD(call), IPC_GET_ARG1(call), IPC_GET_ARG2(call),
	               IPC_GET_ARG3(call), IPC_GET_ARG4(call), IPC_GET_ARG5(call));
	call.callid = callid;
*/
	return call;
}


Native_ipc_call Spartan::ipc_wait_for_call_timeout(Genode::addr_t usec)
{
	Native_ipc_call call;

	do {
		call = ipc_wait_cycle(usec, SYNCH_FLAGS_NONE);
	} while (!call.callid);
/* DEBUG
	Genode::printf("Spartan::ipc_wait_for_call_timeout:\treceived: callid=%lu, "
	               "in_task_id=%lu, in_phonehash=%lu\t IMETHOD=%lu, "
	               "ARG1=%lu, ARG2=%lu, ARG3=%lu, ARG4=%lu, ARG5=%lu\t my_thread_id=%lu\n",
	               call.callid, call.in_task_id, call.in_phone_hash,
	               IPC_GET_IMETHOD(call), IPC_GET_ARG1(call), IPC_GET_ARG2(call),
	               IPC_GET_ARG3(call), IPC_GET_ARG4(call), IPC_GET_ARG5(call),
	               Spartan::thread_get_id());
*/
	return call;
}


Native_ipc_call Spartan::ipc_trywait_for_call()
{
	Native_ipc_call call;

	do {
		call = ipc_wait_cycle(SYNCH_NO_TIMEOUT,
		                      SYNCH_FLAGS_NON_BLOCKING);
	} while (!call.callid);

	return call;
}



/********************
 * Forward requests *
 ********************/

addr_t Spartan::ipc_forward_fast(addr_t callid, int phoneid, addr_t imethod,
                     addr_t arg1, addr_t arg2, unsigned int mode)
{
	return __SYSCALL6(SYS_IPC_FORWARD_FAST, callid, phoneid, imethod, arg1,
	                  arg2, mode);
}


addr_t Spartan::ipc_forward_slow(addr_t callid, int phoneid, addr_t imethod,
                     addr_t arg1, addr_t arg2, addr_t arg3, addr_t arg4,
                     addr_t arg5, unsigned int mode)
{
	Native_ipc_call native_call;

	IPC_SET_IMETHOD(native_call, imethod);
	IPC_SET_ARG1(native_call, arg1);
	IPC_SET_ARG2(native_call, arg2);
	IPC_SET_ARG3(native_call, arg3);
	IPC_SET_ARG4(native_call, arg4);
	IPC_SET_ARG5(native_call, arg5);

	return __SYSCALL4(SYS_IPC_FORWARD_SLOW, callid, phoneid,
	                  (sysarg_t) &native_call, mode);
}



/*******************
 * Answer requests *
 *******************/

addr_t Spartan::ipc_answer_fast(addr_t callid, addr_t retval, addr_t arg1,
                       addr_t arg2, addr_t arg3, addr_t arg4)
{
	return __SYSCALL6(SYS_IPC_ANSWER_FAST, callid, retval, arg1, arg2, arg3,
	                  arg4);
}


addr_t Spartan::ipc_answer_slow(addr_t callid, addr_t retval, addr_t arg1,
                       addr_t arg2, addr_t arg3, addr_t arg4, addr_t arg5)
{
	Native_ipc_call call;

	IPC_SET_RETVAL(call, retval);
	IPC_SET_ARG1(call, arg1);
	IPC_SET_ARG2(call, arg2);
	IPC_SET_ARG3(call, arg3);
	IPC_SET_ARG4(call, arg4);
	IPC_SET_ARG5(call, arg5);

	return __SYSCALL2(SYS_IPC_ANSWER_SLOW, callid, (addr_t) &call);
}



/*************************
 * Connection management *
 *************************/

addr_t Spartan::ipc_connect_me_to(int phoneid, Native_thread_id dest_threadid,
                                  Native_thread_id my_threadid)
{
	return ipc_call_async_fast(phoneid, IPC_M_CONNECT_ME_TO, 0, 0,
	                           my_threadid, dest_threadid);
}


addr_t Spartan::ipc_connect_to_me(int phoneid, Native_thread_id dest_threadid,
                                  Native_thread_id my_threadid)
{
	return ipc_call_async_fast(phoneid, IPC_M_CONNECT_TO_ME, 0, 0,
	                           my_threadid, dest_threadid);
/*
	Genode::printf("Spartan::ipc_connect_to_me:\treceived: callid=%lu, "
	               "in_task_id=%lu, in_phonehash=%lu\t IMETHOD=%lu, "
	               "ARG1=%lu, ARG2=%lu, ARG3=%lu, ARG4=%lu, ARG5=%lu\n",
	               call.callid, call.in_task_id, call.in_phone_hash,
	               IPC_GET_IMETHOD(call), IPC_GET_ARG1(call), IPC_GET_ARG2(call),
	               IPC_GET_ARG3(call), IPC_GET_ARG4(call), IPC_GET_ARG5(call));
*/
}


addr_t Spartan::ipc_send_phone(int snd_phone, int clone_phone, addr_t local_name,
                               Native_thread_id target_threadid,
                               Native_thread_id dest_threadid,
                               Native_thread_id my_threadid)
{
	return ipc_call_async_slow(snd_phone, IPC_M_CONNECTION_CLONE,
	                           (addr_t) clone_phone, target_threadid,
	                           my_threadid, dest_threadid, local_name);
}



/****************
 * Hangup phone *
 ****************/

int Spartan::ipc_hangup(int phoneid, Native_thread_id dest_threadid,
                        Native_thread_id my_threadid)
{
	return __SYSCALL5(SYS_IPC_HANGUP, phoneid, 0, 0, my_threadid, dest_threadid);
}


/********************************
 * Send & receive buffered data *
 ********************************/

Native_ipc_callid Spartan::ipc_data_write(int snd_phone, const void* data,
                                                  size_t size,
                                                  Native_thread_id dst_threadid,
                                                  Native_thread_id my_threadid)
{
	return ipc_call_async_fast(snd_phone, IPC_M_DATA_WRITE, (addr_t)data, size,
	                           my_threadid, dst_threadid);
}

addr_t Spartan::ipc_data_write_accept(addr_t callid, void* data, size_t size,
                                      Native_thread_id snd_threadid)
{
	return ipc_answer_2(callid, snd_threadid, EOK, (addr_t)data, size);
}

Native_ipc_callid Spartan::ipc_data_read(int snd_phone, void* data, size_t size,
                                        Native_thread_id dst_threadid,
                                        Native_thread_id my_threadid)
{
	return ipc_call_async_fast(snd_phone, IPC_M_DATA_READ, (addr_t)data, size,
	                           my_threadid, dst_threadid);
}

addr_t Spartan::ipc_data_read_accept(addr_t callid, const void* data,
                                     size_t size, Native_thread_id snd_threadid)
{
	return ipc_answer_2(callid, snd_threadid, EOK, (addr_t)data, size);
}
