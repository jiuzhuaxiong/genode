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

Native_thread_id Spartan::thread_get_id(void)
{
	Native_thread_id thread_id;

	(void) __SYSCALL1(SYS_THREAD_GET_ID, (sysarg_t) &thread_id);

	return thread_id;
}

Native_thread Spartan::thread_create(void *ip, void *sp, addr_t stack_size,
		const char *name)
{
	uspace_arg_t uarg;
	Native_thread tid;
	int rc;

	uarg.uspace_entry = ip;
	uarg.uspace_stack = sp;
	uarg.uspace_stack_size = stack_size;
	uarg.uspace_uarg = &uarg;

	rc = __SYSCALL4(SYS_THREAD_CREATE, (addr_t) &uarg, (addr_t) name,
			(addr_t) Genode::strlen(name), (addr_t) &tid);

	return rc? INVALID_ID : tid;
}

Native_ipc_callid Spartan::ipc_wait_cycle(Native_ipc_call *call, addr_t usec, 
		unsigned int flags)
{
	Native_ipc_callid callid =
		__SYSCALL3(SYS_IPC_WAIT, (addr_t) call, usec, flags);

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

int Spartan::ipc_connect_to_me(int phoneid, Native_thread_id my_threadid,
		addr_t arg2, addr_t arg3, Native_task *task_id, 
		addr_t *phonehash)
{
	Native_ipc_call data;
	int rc = __SYSCALL6(SYS_IPC_CALL_SYNC_FAST, phoneid,
			IPC_M_CONNECT_TO_ME, my_threadid, arg2, arg3, 
			(addr_t) &data);
	if (rc == 0) {
		*task_id = data.in_task_id;
		*phonehash = IPC_GET_ARG5(data);
	}
	return rc;
}

int Spartan::ipc_connect_me_to(int phoneid, addr_t dest_task_id, 
		Native_thread_id dest_threadid, Native_thread_id my_threadid)
{
	addr_t newphid;
	int res = ipc_call_sync_3_5(phoneid, IPC_M_CONNECT_ME_TO, my_threadid,
			dest_task_id, dest_threadid, 0, 0, 0, 0, &newphid);
	if (res)
		return res;

	return newphid;
}

int Spartan::ipc_clone_connection(int phoneid, Genode::Native_task dst_task_id,
		Genode::Native_thread_id dst_thread_id, long cap_id,
		int clone_phone)
{
	return ipc_call_async_fast(phoneid, IPC_M_CONNECTION_CLONE,
			(addr_t)clone_phone, dst_task_id, dst_thread_id,
			(addr_t) cap_id);
}

/**************************
 * Synchronous Framework *
 **************************/

/** Fast synchronous call.
 *
 * Only three payload arguments can be passed using this function. However,
 * this function is faster than the generic ipc_call_sync_slow() because
 * the payload is passed directly in registers.
 *
 * @param phoneid Phone handle for the call.
 * @param method  Requested method.
 * @param arg1    Service-defined payload argument.
 * @param arg2    Service-defined payload argument.
 * @param arg3    Service-defined payload argument.
 * @param result1 If non-NULL, the return ARG1 will be stored there.
 * @param result2 If non-NULL, the return ARG2 will be stored there.
 * @param result3 If non-NULL, the return ARG3 will be stored there.
 * @param result4 If non-NULL, the return ARG4 will be stored there.
 * @param result5 If non-NULL, the return ARG5 will be stored there.
 *
 * @return Negative values representing IPC errors.
 * @return Otherwise the RETVAL of the answer.
 *
 */
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

/** Synchronous call transmitting 5 arguments of payload.
 *
 * @param phoneid Phone handle for the call.
 * @param imethod Requested interface and method.
 * @param arg1    Service-defined payload argument.
 * @param arg2    Service-defined payload argument.
 * @param arg3    Service-defined payload argument.
 * @param arg4    Service-defined payload argument.
 * @param arg5    Service-defined payload argument.
 * @param result1 If non-NULL, storage for the first return argument.
 * @param result2 If non-NULL, storage for the second return argument.
 * @param result3 If non-NULL, storage for the third return argument.
 * @param result4 If non-NULL, storage for the fourth return argument.
 * @param result5 If non-NULL, storage for the fifth return argument.
 *
 * @return Negative values representing IPC errors.
 * @return Otherwise the RETVAL of the answer.
 *
 */
int Spartan::ipc_call_sync_slow(int phoneid, addr_t method, addr_t arg1,
		addr_t arg2, addr_t arg3, addr_t arg4, addr_t arg5,
		addr_t *result1, addr_t *result2, addr_t *result3,
		addr_t *result4, addr_t *result5)
{
	Native_ipc_call resdata;

	IPC_SET_IMETHOD(resdata, method);
	IPC_SET_ARG1(resdata, arg1);
	IPC_SET_ARG2(resdata, arg2);
	IPC_SET_ARG3(resdata, arg3);
	IPC_SET_ARG4(resdata, arg4);
	IPC_SET_ARG5(resdata, arg5);

	int callres = __SYSCALL3(SYS_IPC_CALL_SYNC_SLOW, phoneid, 
		(sysarg_t) &resdata, (sysarg_t) &resdata);
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

/*************************
 * Asynchronous Framwork *
 *************************/

/** Fast asynchronous call.
 *
 * This function can only handle four arguments of payload. It is, however,
 * faster than the more generic ipc_call_async_slow().
 *
 * Note that this function is a void function.
 *
 * During normal operation, answering this call will trigger the callback.
 * In case of fatal error, the callback handler is called with the proper
 * error code. If the call cannot be temporarily made, it is queued.
 *
 * @param phoneid     Phone handle for the call.
 * @param imethod     Requested interface and method.
 * @param arg1        Service-defined payload argument.
 * @param arg2        Service-defined payload argument.
 * @param arg3        Service-defined payload argument.
 * @param arg4        Service-defined payload argument.
 * @param private     Argument to be passed to the answer/error callback.
 * @param callback    Answer or error callback.
 * @param can_preempt If true, the current fibril will be preempted in
 *                    case the kernel temporarily refuses to accept more
 *                    asynchronous calls.
 *
 */
Native_ipc_callid Spartan::ipc_call_async_fast(int phoneid, addr_t imethod, addr_t arg1,
		addr_t arg2, addr_t arg3, addr_t arg4)
//, void *priv, ipc_async_callback_t callback, bool can_preempt)
{
	Native_ipc_callid callid = __SYSCALL6(SYS_IPC_CALL_ASYNC_FAST, phoneid,
			imethod, arg1, arg2, arg3, arg4);

	return callid;
}

/***************
 * IPC answers *
 ***************/
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

int Spartan::ipc_data_write_start_synch(int phoneid, Native_task dst_task,
		Native_thread_id dst_thread, const void *src, size_t size)
{
	return ipc_call_sync_5_0(phoneid, IPC_M_DATA_WRITE, (sysarg_t) src,
			(sysarg_t) size, (sysarg_t) dst_task,
			(sysarg_t) dst_thread, thread_get_id());
}

/* SHOULD NOT BE USED ANYMORE */
bool Spartan::ipc_data_write_receive_timeout(Native_ipc_callid *callid,
		Native_ipc_call *call, Native_thread_id *in_thread_id,
		addr_t *size, addr_t usec)
{
	*callid = ipc_wait_for_call_timeout(call, usec);
	/*TODO dummy until handled correctly */
	*in_thread_id = 0;

	if (IPC_GET_IMETHOD(*call) != IPC_M_DATA_WRITE)
		return false;

	if (size)
		*size = (addr_t) IPC_GET_ARG2(*call);

	return true;
}

int Spartan::ipc_data_write_finalize(Native_ipc_callid callid, void *dst, addr_t size)
{
	return ipc_answer_2(callid, EOK, (sysarg_t) dst, (sysarg_t) size);
}

int Spartan::ipc_hangup(int phoneid)
{
	return __SYSCALL1(SYS_IPC_HANGUP, phoneid);
}

/***************
 * Futex calls *
 ***************/

int Spartan::futex_sleep(volatile int *futex)
{
	return __SYSCALL1(SYS_FUTEX_SLEEP, (sysarg_t) futex);
}

int Spartan::futex_wakeup(volatile int *futex)
{
	return __SYSCALL1(SYS_FUTEX_WAKEUP, (sysarg_t) futex);
}


/*8*************
 * Timer calls *
 ***************/
int Spartan::usleep(addr_t usec)
{
	(void) __SYSCALL1(SYS_THREAD_USLEEP, usec);
	return 0;
}

void Spartan::udelay(addr_t time)
{
	(void) __SYSCALL1(SYS_THREAD_UDELAY, (sysarg_t) time);
}

void Spartan::exit(int status)
{
	__SYSCALL1(SYS_TASK_EXIT, false);
	/* Unreachable */
	while (1);
}
