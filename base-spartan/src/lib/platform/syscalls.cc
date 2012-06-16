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

extern "C" addr_t __syscall(const addr_t p1, const addr_t p2,
	const addr_t p3, const addr_t p4, const addr_t p5, const addr_t p6,
	const syscall_t id);


void Spartan::io_port_enable(Genode::addr_t pio_addr, Genode::size_t size)
{
	ddi_ioarg_t	arg;

	arg.task_id = task_get_id();
	arg.ioaddr = (void*)pio_addr;
	arg.size = size;

	__SYSCALL1(SYS_IOSPACE_ENABLE, (addr_t) &arg);
}


Native_task Spartan::task_get_id(void)
{
	Native_task task_id;
#ifdef __32_BITS
	(void) __SYSCALL1(SYS_TASK_GET_ID, (addr_t) &task_id);
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

	rc = __SYSCALL4(SYS_THREAD_CREATE, (addr_t) &uarg, (addr_t) name,
			(addr_t) Genode::strlen(name), (addr_t) &tid);

	return rc? INVALID_THREAD_ID : tid;
}

Native_ipc_callid Spartan::ipc_wait_cycle(Native_ipc_call *call, addr_t usec, 
		unsigned int flags)
{
	Native_ipc_callid callid =
		__SYSCALL3(SYS_IPC_WAIT, (addr_t) call, usec, flags);

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

int Spartan::ipc_connect_me_to(int phoneid, addr_t arg1, addr_t arg2,
		addr_t arg3)
{
	addr_t newphid;
	int res = ipc_call_sync_3_5(phoneid, IPC_M_CONNECT_ME_TO, arg1, arg2, arg3,
			0, 0, 0, 0, &newphid);
//			NULL, NULL, NULL, NULL, &newphid);
	if (res)
	return res;

	return newphid;
}

int Spartan::ipc_call_sync_fast(int phoneid, addr_t method, addr_t arg1,
		addr_t arg2, addr_t arg3, addr_t *result1, addr_t *result2,
		addr_t *result3, addr_t *result4, addr_t *result5)
{
	Native_ipc_call resdata;
	int callres = __SYSCALL6(SYS_IPC_CALL_SYNC_FAST, phoneid, method, arg1,
			arg2, arg3, (addr_t) &resdata);
	if (callres)
		return callres;

	if (result1)
		*result1 = IPC_GET_ARG1(resdata);
	if (result2)
		*result2 = IPC_GET_ARG2(resdata);
	if (result3)
		*result3 = IPC_GET_ARG3(resdata);
	if (result4)
		*result4 = IPC_GET_ARG4(resdata);
	if (result5)
		*result5 = IPC_GET_ARG5(resdata);

	return IPC_GET_RETVAL(resdata);
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

	return __SYSCALL2(SYS_IPC_ANSWER_SLOW, callid, (addr_t) &call);
}

int Spartan::ipc_forward_fast(Native_ipc_callid callid, int phoneid,
		addr_t imethod, addr_t arg1,
		addr_t arg2, unsigned int mode)
{
	return __SYSCALL6(SYS_IPC_FORWARD_FAST, callid, phoneid, imethod, arg1,
		arg2, mode);
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
