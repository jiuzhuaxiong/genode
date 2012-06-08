#include <spartan/syscalls.h>
#include <util/string.h>

using namespace Genode;

extern "C" {
#include <sys/types.h>
#include <abi/ddi/arg.h>
#include <abi/syscall.h>
#include <abi/proc/uarg.h>
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



void Spartan::exit(int status)
{
	__SYSCALL1(SYS_TASK_EXIT, false);
	/* Unreachable */
	while (1);
}
