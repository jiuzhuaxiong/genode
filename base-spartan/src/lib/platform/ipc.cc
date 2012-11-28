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

Native_ipc_callid Spartan::ipc_call_async_fast(int phoneid, addr_t imethod,
                                               addr_t arg1, addr_t arg2,
                                               addr_t arg3, addr_t arg4,
                                               Native_ipc_call *retcall)
{
	Native_ipc_callid callid;
	callid = __SYSCALL6(SYS_IPC_CALL_ASYNC_FAST, phoneid, imethod, arg1,
	                    arg2, arg3, arg4);

	if (callid == (Native_ipc_callid) IPC_CALLRET_TEMPORARY) {
		retcall = new Native_ipc_call();

		IPC_SET_IMETHOD(*retcall, imethod);
		IPC_SET_ARG1(*retcall, arg1);
		IPC_SET_ARG2(*retcall, arg2);
		IPC_SET_ARG3(*retcall, arg3);
		IPC_SET_ARG4(*retcall, arg4);
		IPC_SET_ARG5(*retcall, 0);
	}

	return callid;
}


Native_ipc_callid Spartan::ipc_call_async_slow(int phoneid, addr_t imethod,
                                               addr_t arg1, addr_t arg2,
                                               addr_t arg3, addr_t arg4,
                                               addr_t arg5,
                                               Native_ipc_call *retcall)
{
	/**
	 * TODO: Is it safe to use a non-pointer here?
	 * Does the kernel copy the variable call into the adress sapce of the 
	 *  other thread or will it just set the pointer to the physical adress 
	 *  of my var call?
	 */
	Native_ipc_call call;
	IPC_SET_IMETHOD(call, imethod);
	IPC_SET_ARG1(call, arg1);
	IPC_SET_ARG2(call, arg2);
	IPC_SET_ARG3(call, arg3);
	IPC_SET_ARG4(call, arg4);
	IPC_SET_ARG5(call, arg5);

	Native_ipc_callid callid;
	callid =  __SYSCALL2(SYS_IPC_CALL_ASYNC_SLOW, phoneid,
	                     (sysarg_t) &call);


	if (callid == (Native_ipc_callid) IPC_CALLRET_TEMPORARY) {
		retcall = new Native_ipc_call(call);
	}

	return callid;
}



/*******************************
 * Wait for incomming requests *
 *******************************/

Native_ipc_callid Spartan::ipc_wait_cycle(Native_ipc_call *call, addr_t usec,
                                 unsigned int flags)
{
	Native_ipc_callid callid = __SYSCALL3(SYS_IPC_WAIT, (addr_t) call, usec,
	                                      flags);

	return callid;
}


Native_ipc_callid Spartan::ipc_wait_for_call_timeout(Native_ipc_call *call,
                                            Genode::addr_t usec)
{
	Native_ipc_callid callid;

	do {
		callid = ipc_wait_cycle(call, usec, SYNCH_FLAGS_NONE);
	} while (callid & IPC_CALLID_ANSWERED);

	return callid;
}


Native_ipc_callid Spartan::ipc_trywait_for_call(Native_ipc_call *call)
{
	Native_ipc_callid callid;

	do {
		callid = ipc_wait_cycle(call, SYNCH_NO_TIMEOUT,
		                        SYNCH_FLAGS_NON_BLOCKING);
	} while (callid & IPC_CALLID_ANSWERED);

	return callid;
}



/********************
 * Forward requests *
 ********************/

int Spartan::ipc_forward_fast(Native_ipc_callid callid, int phoneid, addr_t imethod,
                     addr_t arg1, addr_t arg2, unsigned int mode)
{
	return __SYSCALL6(SYS_IPC_FORWARD_FAST, callid, phoneid, imethod, arg1,
	                  arg2, mode);
}


int Spartan::ipc_forward_slow(Native_ipc_callid callid, int phoneid, addr_t imethod,
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

addr_t Spartan::ipc_answer_fast(Native_ipc_callid callid, addr_t retval, addr_t arg1,
                       addr_t arg2, addr_t arg3, addr_t arg4)
{
	return __SYSCALL6(SYS_IPC_ANSWER_FAST, callid, retval, arg1, arg2, arg3,
	                  arg4);
}


addr_t Spartan::ipc_answer_slow(Native_ipc_callid callid, addr_t retval, addr_t arg1,
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

int ipc_connect_me_to(int phoneid, Native_thread_id dest_threadid,
                      Native_thread_id my_threadid)
{
	Native_ipc_call *retdata = 0;
	int rc = ipc_call_async_fast(phoneid, IPC_M_CONNECT_ME_TO,
	                             dest_threadid, my_threadid, 0, 0, retdata);

	/* FIXME
	 * WH000000t?
	 */

	return rc;
}


int ipc_connect_to_me(int phoneid, Native_task *task_id, addr_t *phonehash)
{
	/* TODO
	 * workaround needed, since messages can only be send to tasks,
	 * Genode meanwhile needs to adress threads.
	 */
	Native_ipc_call data;
	int rc = ipc_call_async_fast(phoneid, IPC_M_CONNECT_TO_ME, 0, 0, 0,
	                             (addr_t) &data, 0);
	if (rc == 0) {
		*task_id = data.in_task_id;
		*phonehash = IPC_GET_ARG5(data);
	}

	return rc;
}


int ipc_send_phone(int snd_phone, int clone_phone)
{
	/**
	 * TODO
	 * workaround needed, since messages can only be send to tasks,
	 * Genode meanwhile needs to adress threads.
	 */
	return ipc_call_async_fast(snd_phone, IPC_M_CONNECTION_CLONE, 0, 0, 0,
	                           (addr_t) clone_phone, 0);
}



/****************
 * Hangup phone *
 ****************/

int Spartan::ipc_hangup(int phoneid)
{
	return __SYSCALL1(SYS_IPC_HANGUP, phoneid);
}
