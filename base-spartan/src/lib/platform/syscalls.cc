#include <spartan/syscalls.h>

using namespace Genode;

extern "C" {
#include <sys/types.h>
#include <abi/ddi/arg.h>
#include <abi/syscall.h>
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

task_id_t Spartan::task_get_id(void)
{
#ifdef __32_BITS
	task_id_t task_id;
	(void) __SYSCALL1(SYS_TASK_GET_ID, (sysarg_t) &task_id);
	return task_id;
#endif  /* __32_BITS__ */

#ifdef __64_BITS__
	return (task_id_t) __SYSCALL0(SYS_TASK_GET_ID);
#endif  /* __64_BITS__ */
}

void Spartan::exit(int status)
{
	__SYSCALL1(SYS_TASK_EXIT, false);
	/* Unreachable */
	while (1);
}
