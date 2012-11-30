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
//for io_port_enable()
#include <abi/ddi/arg.h>
//#include <abi/syscall.h>
//for thread_create()
#include <abi/proc/uarg.h>
}
//#include <sys/types.h>
//#include <abi/ipc/ipc.h>
//#include <abi/ipc/methods.h>
//#include <abi/synch.h>
//#include <abi/mm/as.h>


using namespace Spartan;


/*************************
 * Basic functionalities *
 ************************/

extern "C" int _main();


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
#ifdef __32_BITS__
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

	return rc? rc : tid;
}


