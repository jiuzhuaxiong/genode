#include <spartan/syscalls.h>
#include <util/string.h>

using namespace Genode;

extern "C" {
#include <sys/types.h>
#include <abi/ddi/arg.h>
#include <abi/syscall.h>
#include <abi/proc/uarg.h>
#include <abi/ipc/ipc.h>
#include <abi/ipc/methods.h>
#include <abi/synch.h>
}

using namespace Spartan;

extern "C" sysarg_t __syscall(const sysarg_t p1, const sysarg_t p2,
	const sysarg_t p3, const sysarg_t p4, const sysarg_t p5, const sysarg_t p6,
	const syscall_t id);


void Spartan::io_port_enable(Genode::addr_t pio_addr, Genode::size_t size)
{
	ddi_ioarg_t	arg;

	arg.task_id = task_get_id();
	arg.ioaddr = (void*)pio_addr;
	arg.size = size;

	__SYSCALL1(SYS_IOSPACE_ENABLE, (sysarg_t) &arg);
}


Native_task Spartan::task_get_id(void)
{
	Native_task task_id;
#ifdef __32_BITS
	(void) __SYSCALL1(SYS_TASK_GET_ID, (sysarg_t) &task_id);
#endif  /* __32_BITS__ */

#ifdef __64_BITS__
	task_id = (Native_task) __SYSCALL0(SYS_TASK_GET_ID);
#endif  /* __64_BITS__ */
	return task_id;
}

Native_thread Spartan::thread_create(void *ip, void *sp, const char *name)
{
	uspace_arg_t uarg;
	Native_thread tid;
	int rc;

	uarg.uspace_entry = ip;
	uarg.uspace_stack = sp;
	uarg.uspace_uarg = &uarg;

	rc = __SYSCALL4(SYS_THREAD_CREATE, (sysarg_t) &uarg, (sysarg_t) name,
			(sysarg_t) Genode::strlen(name), (sysarg_t) &tid);

	return rc? INVALID_THREAD_ID : tid;
}

Native_ipc_callid Spartan::ipc_wait_cycle(Native_ipc_call *call, addr_t usec, 
		unsigned int flags)
{
	Native_ipc_callid callid =
		__SYSCALL3(SYS_IPC_WAIT, (sysarg_t) call, usec, flags);

	/* Handle received answers */
	/* TODO ?
	if (callid & IPC_CALLID_ANSWERED) {
		handle_answer(callid, call);
		dispatch_queued_calls();
	}
	*/
	return callid;
}

Native_ipc_callid Spartan::ipc_wait_for_call_timeout(Native_ipc_call *call, addr_t usec)
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

int Spartan::ipc_connect_to_me(int phoneid, addr_t arg1, addr_t arg2, 
		addr_t arg3, Native_task *task_id, addr_t *phonehash)
{
	Native_ipc_call data;
	int rc = __SYSCALL6(SYS_IPC_CALL_SYNC_FAST, phoneid,
			IPC_M_CONNECT_TO_ME, arg1, arg2, arg3, (addr_t) &data);
	if (rc == 0) {
		*task_id = data.in_task_id;
		*phonehash = IPC_GET_ARG5(data);
	}
	return rc;
}

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

	return __SYSCALL2(SYS_IPC_ANSWER_SLOW, callid, (sysarg_t) &call);
}

int Spartan::ipc_hangup(int phoneid)
{
	return __SYSCALL1(SYS_IPC_HANGUP, phoneid);
}

void Spartan::exit(int status)
{
	__SYSCALL1(SYS_TASK_EXIT, false);
	/* Unreachable */
	while (1);
}
