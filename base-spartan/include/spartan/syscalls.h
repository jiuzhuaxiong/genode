/*
 * \brief  Spartan syscalls
 * \author Tobias Börtitz
 * \date   2012-08-14
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__SPARTAN__SYSCALLS_H_
#define _INCLUDE__SPARTAN__SYSCALLS_H_

#include <base/stdint.h>
#include <base/native_types.h>

namespace Spartan
{

#include <sys/types.h>
#include <abi/ipc/ipc.h>
#include <abi/ipc/methods.h>
#include <abi/mm/as.h>
#include <abi/errno.h>

#define __SYSCALL0(id) \
	__syscall(0, 0, 0, 0, 0, 0, id)
#define __SYSCALL1(id, p1) \
	__syscall(p1, 0, 0, 0, 0, 0, id)
#define __SYSCALL2(id, p1, p2) \
	__syscall(p1, p2, 0, 0, 0, 0, id)
#define __SYSCALL3(id, p1, p2, p3) \
	__syscall(p1, p2, p3, 0, 0, 0, id)
#define __SYSCALL4(id, p1, p2, p3, p4) \
	__syscall(p1, p2, p3, p4, 0, 0, id)
#define __SYSCALL5(id, p1, p2, p3, p4, p5) \
	__syscall(p1, p2, p3, p4, p5, 0, id)
#define __SYSCALL6(id, p1, p2, p3, p4, p5, p6) \
	__syscall(p1, p2, p3, p4, p5, p6, id)

#define ipc_call_sync_2_0(phoneid, method, arg1, arg2) \
	ipc_call_sync_fast((phoneid), (method), (arg1), (arg2), 0, 0, 0, 0, \
		0, 0)
#define ipc_call_sync_3_0(phoneid, method, arg1, arg2, arg3) \
	ipc_call_sync_fast((phoneid), (method), (arg1), (arg2), (arg3), 0, 0, 0, \
		0, 0)
#define ipc_call_sync_2_5(phoneid, method, arg1, arg2, res1, res2, \
		res3, res4, res5) \
	ipc_call_sync_fast((phoneid), (method), (arg1), (arg2), 0, \
		(res1), (res2), (res3), (res4), (res5))
#define ipc_call_sync_3_5(phoneid, method, arg1, arg2, arg3, res1, res2, \
		res3, res4, res5) \
	ipc_call_sync_fast((phoneid), (method), (arg1), (arg2), (arg3), \
		(res1), (res2), (res3), (res4), (res5))
#define ipc_call_sync_4_0(phoneid, method, arg1, arg2, arg3, arg4) \
	ipc_call_sync_slow((phoneid), (method), (arg1), (arg2), (arg3), (arg4), \
		0, 0, 0, 0, 0, 0)
#define ipc_call_sync_5_0(phoneid, method, arg1, arg2, arg3, arg4, arg5) \
	ipc_call_sync_slow((phoneid), (method), (arg1), (arg2), (arg3), (arg4), \
		(arg5), 0, 0, 0, 0, 0)

#define ipc_call_async_2(phoneid, method, arg1, arg2) \
	ipc_call_async_fast((phoneid), (method), (arg1), (arg2), 0, 0)
#define ipc_call_async_5_0(phoneid, method, arg1, arg2, arg3, arg4, arg5) \
	ipc_call_async_slow((phoneid), (method), (arg1), (arg2), (arg3), (arg4), \
		(arg5))

/*
 * User-friendly wrappers for ipc_answer_fast() and ipc_answer_slow().
 * They are in the form of ipc_answer_m(), where m is the number of return
 * arguments. The macros decide between the fast and the slow version according
 * to m.
 */
#define ipc_answer_0(callid, retval) \
	ipc_answer_fast((callid), (retval), 0, 0, 0, 0)
#define ipc_answer_1(callid, retval, arg1) \
	ipc_answer_fast((callid), (retval), (arg1), 0, 0, 0)
#define ipc_answer_2(callid, retval, arg1, arg2) \
	ipc_answer_fast((callid), (retval), (arg1), (arg2), 0, 0)
#define ipc_answer_3(callid, retval, arg1, arg2, arg3) \
	ipc_answer_fast((callid), (retval), (arg1), (arg2), (arg3), 0)
#define ipc_answer_4(callid, retval, arg1, arg2, arg3, arg4) \
	ipc_answer_fast((callid), (retval), (arg1), (arg2), (arg3), (arg4))
#define ipc_answer_5(callid, retval, arg1, arg2, arg3, arg4, arg5) \
	ipc_answer_slow((callid), (retval), (arg1), (arg2), (arg3), (arg4), (arg5))

	/*************************
	 * Basic functionalities *
	 ************************/

	void exit(int status);

	void io_port_enable(Genode::addr_t pio_addr, Genode::size_t size);

	Genode::Native_task task_get_id(void);
	Genode::Native_thread_id thread_get_id(void);
	Genode::Native_thread thread_create(void* ip, void* sp, 
			Genode::addr_t stack_size, const char* name);
}

#endif /* _INCLUDE__SPARTAN__SYSCALLS_H_ */
