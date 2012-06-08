#ifndef SPARTAN_SYSCALLS
#define SPARTAN_SYSCALLS

#include <base/stdint.h>

namespace Spartan
{

#include <sys/types.h>

typedef uint64_t task_id_t;

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
	extern "C" sysarg_t __syscall(const sysarg_t, const sysarg_t, const sysarg_t,
		const sysarg_t, const sysarg_t, const sysarg_t, const syscall_t);
	*/
	/*
	extern "C" sysarg_t __syscall(const sysarg_t p1, const sysarg_t p2,
		const sysarg_t p3, const sysarg_t p4, const sysarg_t p5, const sysarg_t p6,
		const syscall_t id);
*/
	void io_port_enable(Genode::addr_t pio_addr, Genode::size_t size);
	void exit(int status);

	task_id_t task_get_id(void);
}

#endif /* SPARTAN_SYSCALL */
