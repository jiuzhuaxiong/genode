#ifndef SPARTAN_SYSCALLS
#define SPARTAN_SYSCALLS

#include <base/stdint.h>
#include <base/native_types.h>

namespace Spartan
{

#include <sys/types.h>
#include <abi/ipc/ipc.h>
#include <abi/ipc/methods.h>
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


	void io_port_enable(Genode::addr_t pio_addr, Genode::size_t size);
	void exit(int status);

	Genode::Native_task task_get_id(void);
	Genode::Native_thread thread_create(void* ip, void* sp,
			const char* name);

	/** Wait for first IPC call to come. */
	Genode::Native_ipc_callid ipc_wait_cycle(Genode::Native_ipc_call *call,
			Genode::addr_t usec, unsigned int flags);
	/** Wait for first IPC call to come. */
	Genode::Native_ipc_callid 
		ipc_wait_for_call_timeout(Genode::Native_ipc_call *call, 
				Genode::addr_t usec);
	/** Check if there is an IPC call waiting to be picked up. */
	Genode::Native_ipc_callid 
		ipc_trywait_for_call(Genode::Native_ipc_call *call);

	/** Request new connection. */
	int ipc_connect_to_me(int phoneid, Genode::addr_t arg1,
			Genode::addr_t arg2, Genode::addr_t arg3,
			Genode::Native_task *task_id,
			Genode::addr_t *phonehash);

	/** Answer received call (fast version).
	 * @return Zero on success.
	 * @return Value from @ref errno.h on failure.
	 */
	Genode::addr_t ipc_answer_fast(Genode::Native_ipc_callid callid,
			Genode::addr_t retval, Genode::addr_t arg1,
			Genode::addr_t arg2, Genode::addr_t arg3,
			Genode::addr_t arg4);
	/** Answer received call (entire payload).
	 * @return Zero on success.
	 * @return Value from @ref errno.h on failure.
	 */
	Genode::addr_t ipc_answer_slow(Genode::Native_ipc_callid callid,
			Genode::addr_t retval, Genode::addr_t arg1,
			Genode::addr_t arg2, Genode::addr_t arg3,
			Genode::addr_t arg4, Genode::addr_t arg5);


	/** Hang up a phone. */
	int ipc_hangup(int phoneid);
}

#endif /* SPARTAN_SYSCALL */
