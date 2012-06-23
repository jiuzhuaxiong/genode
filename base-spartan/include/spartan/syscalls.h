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

#define ipc_call_sync_2_0(phoneid, method, arg1, arg2) \
	ipc_call_sync_fast((phoneid), (method), (arg1), (arg2), 0, 0, 0, 0, \
		0, 0)
#define ipc_call_sync_3_5(phoneid, method, arg1, arg2, arg3, res1, res2, \
		res3, res4, res5) \
	ipc_call_sync_fast((phoneid), (method), (arg1), (arg2), (arg3), \
		(res1), (res2), (res3), (res4), (res5))

/* ASYNCH */
/*
#define ipc_req_2_0(exch, method, arg1, arg2) \
	ipc_req_fast(exch, method, arg1, arg2, 0, 0, NULL, NULL, NULL, \
		NULL, NULL)
*/

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
	Genode::Native_thread_id thread_get_id(void);
	Genode::Native_thread thread_create(void* ip, void* sp, 
			Genode::addr_t stack_size, const char* name);

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

	/** Request callback connection. */
	int ipc_connect_to_me(int phoneid, Genode::addr_t arg1,
			Genode::addr_t arg2, Genode::addr_t arg3,
			Genode::Native_task *task_id,
			Genode::addr_t *phonehash);
	/** Request new connection. */
	int ipc_connect_me_to(int phoneid, Genode::addr_t dest_task_id,
			Genode::addr_t dest_thread_id, Genode::addr_t arg3);

	/* Fast synchronous call */
	int ipc_call_sync_fast(int phoneid, Genode::addr_t method,
			Genode::addr_t arg1, Genode::addr_t arg2,
			Genode::addr_t arg3, Genode::addr_t *result1,
			Genode::addr_t *result2, Genode::addr_t *result3,
			Genode::addr_t *result4, Genode::addr_t *result5);

	/** Pseudo-synchronous message sending - fast version.
	 * Send message asynchronously and return only after the reply arrives.
	 * TODO problems with asynch_exch_t
	 */
//	Genode::addr_t ipc_req_fast(async_exch_t *exch, sysarg_t imethod, sysarg_t arg1,
//sysarg_t arg2, sysarg_t arg3, sysarg_t arg4, sysarg_t *r1, sysarg_t *r2,
//sysarg_t *r3, sysarg_t *r4, sysarg_t *r5

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

	int ipc_forward_fast(Genode::Native_ipc_callid callid, int phoneid,
			Genode::addr_t imethod, Genode::addr_t arg1,
			Genode::addr_t arg2, unsigned int mode);

	/** Wrappers for IPC_M_DATA_WRITE calls. */
//	int ipc_data_write_start(async_exch_t *exch, const void *src, size_t size);
	/* TODO add thread_id to all operations */
	int ipc_data_write_start_synch(int phoneid, const void *src, size_t size);
	/** Wrapper for receiving the IPC_M_DATA_WRITE calls */
	bool ipc_data_write_receive_timeout(Genode::Native_ipc_callid *callid,
			Genode::Native_ipc_call *call, 
			Genode::Native_thread_id *in_thread_id,
			Genode::addr_t *size, Genode::addr_t usec);
	/** Wrapper for answering the IPC_M_DATA_WRITE calls */
	int ipc_data_write_finalize(Genode::Native_ipc_callid callid,
			void *dst, Genode::addr_t size);

	/** Hang up a phone. */
	int ipc_hangup(int phoneid);
}

#endif /* SPARTAN_SYSCALL */
