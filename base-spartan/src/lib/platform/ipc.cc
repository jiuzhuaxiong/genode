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
//#include <sys/types.h>
//#include <abi/proc/uarg.h>
//#include <abi/ipc/ipc.h>
//#include <abi/ipc/methods.h>
//#include <abi/mm/as.h>


using namespace Spartan;


/**************************************
 *  Send requests & wrapper functions *
 **************************************/

Native_ipc_call Spartan::ipc_call_async_fast(int phoneid, addr_t imethod,
                                               addr_t arg1, addr_t arg2,
                                               addr_t arg3, addr_t arg4)
{
	Native_ipc_call call;
	call.callid = __SYSCALL6(SYS_IPC_CALL_ASYNC_FAST, phoneid, imethod,
	                         arg1, arg2, arg3, arg4);

	/* FIXME
	 * Are the next lines realy needed?
	 * It seems to me as if they are just worthless code :o
	 */
	if (call.callid == (addr_t) IPC_CALLRET_TEMPORARY) {
		IPC_SET_IMETHOD(call, imethod);
		IPC_SET_ARG1(call, arg1);
		IPC_SET_ARG2(call, arg2);
		IPC_SET_ARG3(call, arg3);
		IPC_SET_ARG4(call, arg4);
	}
	else {
		IPC_SET_IMETHOD(call, 0);
		IPC_SET_ARG1(call, 0);
		IPC_SET_ARG2(call, 0);
		IPC_SET_ARG3(call, 0);
		IPC_SET_ARG4(call, 0);
	}
	IPC_SET_ARG5(call, 0);

	return call;
}


Native_ipc_call Spartan::ipc_call_async_slow(int phoneid, addr_t imethod,
                                               addr_t arg1, addr_t arg2,
                                               addr_t arg3, addr_t arg4,
                                               addr_t arg5)
{
	Native_ipc_call call;
	IPC_SET_IMETHOD(call, imethod);
	IPC_SET_ARG1(call, arg1);
	IPC_SET_ARG2(call, arg2);
	IPC_SET_ARG3(call, arg3);
	IPC_SET_ARG4(call, arg4);
	IPC_SET_ARG5(call, arg5);

	call.callid =  __SYSCALL2(SYS_IPC_CALL_ASYNC_SLOW, phoneid,
	                     (sysarg_t) &call);


	/* FIXME
	 * See comment above
	 */
	if (call.callid == (addr_t) IPC_CALLRET_TEMPORARY) {
		IPC_SET_IMETHOD(call, imethod);
		IPC_SET_ARG1(call, arg1);
		IPC_SET_ARG2(call, arg2);
		IPC_SET_ARG3(call, arg3);
		IPC_SET_ARG4(call, arg4);
		IPC_SET_ARG5(call, arg5);
	}

	return call;
}



/*******************************
 * Wait for incomming requests *
 *******************************/

Native_ipc_call Spartan::ipc_wait_cycle(addr_t usec, unsigned int flags)
{
	Native_ipc_call call;
	call.callid = __SYSCALL3(SYS_IPC_WAIT, (addr_t) &call, usec, flags);
/*
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

	Genode::printf("Spartan::ipc_wait_for_call_timeout:\treceived: callid=%lu, "
	               "in_task_id=%lu, in_phonehash=%lu\t IMETHOD=%lu, "
	               "ARG1=%lu, ARG2=%lu, ARG3=%lu, ARG4=%lu, ARG5=%lu\n",
	               call.callid, call.in_task_id, call.in_phone_hash,
	               IPC_GET_IMETHOD(call), IPC_GET_ARG1(call), IPC_GET_ARG2(call),
	               IPC_GET_ARG3(call), IPC_GET_ARG4(call), IPC_GET_ARG5(call));

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

int Spartan::ipc_forward_fast(addr_t callid, int phoneid, addr_t imethod,
                     addr_t arg1, addr_t arg2, unsigned int mode)
{
	return __SYSCALL6(SYS_IPC_FORWARD_FAST, callid, phoneid, imethod, arg1,
	                  arg2, mode);
}


int Spartan::ipc_forward_slow(addr_t callid, int phoneid, addr_t imethod,
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

int Spartan::ipc_connect_me_to(int phoneid, Native_thread_id dest_threadid,
                      Native_thread_id my_threadid)
{
	Native_ipc_call call;
	call = ipc_call_async_fast(phoneid, IPC_M_CONNECT_ME_TO,
	                             dest_threadid, my_threadid, 0, 0);

	/* FIXME
	 * WH000000t?
	 */

	return call.callid;
}


addr_t Spartan::ipc_connect_to_me(int phoneid)
{
	/* TODO
	 * workaround needed, since messages can only be send to tasks,
	 * Genode meanwhile needs to adress threads.
	 */
	Native_ipc_call call;
	call = ipc_call_async_fast(phoneid, IPC_M_CONNECT_TO_ME, 0, 0, 0, 0);
/*
	Genode::printf("Spartan::ipc_connect_to_me:\treceived: callid=%lu, "
	               "in_task_id=%lu, in_phonehash=%lu\t IMETHOD=%lu, "
	               "ARG1=%lu, ARG2=%lu, ARG3=%lu, ARG4=%lu, ARG5=%lu\n",
	               call.callid, call.in_task_id, call.in_phone_hash,
	               IPC_GET_IMETHOD(call), IPC_GET_ARG1(call), IPC_GET_ARG2(call),
	               IPC_GET_ARG3(call), IPC_GET_ARG4(call), IPC_GET_ARG5(call));
*/
	return call.callid;
}


addr_t Spartan::ipc_send_phone(int snd_phone, int clone_phone)
{
	/**
	 * TODO
	 * workaround needed, since messages can only be send to tasks,
	 * Genode meanwhile needs to adress threads.
	 */
	Native_ipc_call call;
	call = ipc_call_async_fast(snd_phone, IPC_M_CONNECTION_CLONE, 0, 0, 0,
	                           (addr_t) clone_phone);

	return call.callid;
}



/****************
 * Hangup phone *
 ****************/

int Spartan::ipc_hangup(int phoneid)
{
	return __SYSCALL1(SYS_IPC_HANGUP, phoneid);
}
