/*
 * \brief  Spartan-specific syscall implemetation
 * \author Tobias Börtitz
 * \date   2012-08-14
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

using namespace Genode;

extern "C" {
#include <sys/types.h>
#include <abi/ddi/arg.h>
#include <abi/syscall.h>
#include <abi/proc/uarg.h>
#include <abi/ipc/ipc.h>
#include <abi/ipc/methods.h>
#include <abi/synch.h>
#include <abi/mm/as.h>
}

using namespace Spartan;

/**
 * TODO
 * remove all usage of thread_get_id()
 * use additional argument instead!
 */


/*************************
 * Basic functionalities *
 ************************/

extern "C" int _main();


extern "C" addr_t __syscall(const addr_t p1, const addr_t p2, const addr_t p3,
                            const addr_t p4, const addr_t p5, const addr_t p6,
                            const syscall_t id);


void Spartan::exit(int status)
{
	__SYSCALL1(SYS_TASK_EXIT, false);
	/* Unreachable */
	while (1);
}


void Spartan::io_port_enable(Genode::addr_t pio_addr, Genode::size_t size)
{
	ddi_ioarg_t	arg;

	arg.task_id = task_get_id();
	arg.ioaddr = (void*)pio_addr;
	arg.size = size;

	__SYSCALL1(SYS_IOSPACE_ENABLE, (addr_t) &arg);
}


/***************************
 * Access to specific ID's *
 **************************/

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
