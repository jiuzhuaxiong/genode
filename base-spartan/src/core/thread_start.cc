/*
 * \brief  Implementation of the core-internal Thread API via Linux threads
 * \author Norman Feske
 * \date   2006-06-13
 */

/*
 * Copyright (C) 2006-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/thread.h>
#include <base/sleep.h>

/* Spartan includes */
#include <spartan/syscalls.h>

using namespace Genode;


static void empty_signal_handler(int) { }


static void thread_start(void *)
{
/*	
	*
	 * Set signal handler such that canceled system calls get not
	 * transparently retried after a signal gets received.
	 *
	lx_sigaction(LX_SIGUSR1, empty_signal_handler);

	*
	 * Prevent children from becoming zombies. (SIG_IGN = 1)
	 *
	lx_sigaction(LX_SIGCHLD, (void (*)(int))1);
*/

	Thread_base::myself()->entry();
	sleep_forever();
	PWRN("%s: Not implemented", __PRETTY_FUNCTION__);
}


void Thread_base::_init_platform_thread()
{
	_tid = Spartan::thread_get_id();
}


void Thread_base::_deinit_platform_thread()
{
	PWRN("%s: Not implemented and not needed", __PRETTY_FUNCTION__);
}


void Thread_base::start()
{
/*
	* align initial stack to 16 byte boundary *
	void *thread_sp = (void *)((addr_t)(_context->stack) & ~0xf);
	_tid.tid = lx_create_thread(thread_start, thread_sp, this);
	_tid.pid = lx_getpid();
*/
	/* FIXME:
	 * since the Spartan kernel wants to know the size of the stack
	 * we have to assume a stack size which is big enough */
	addr_t stack_size = (addr_t) _context->stack - (addr_t) _context->stack_base; //8192;//16384;
//	void* stack_elem = (void*) (_context->stack_base+1024);
	PDBG("*_context=%i, *_context->stack=%i, _context->stack_base=%i, stack_size=%i", _context, _context->stack, _context->stack_base, stack_size);
	PDBG("stack_size = %lu", stack_size);
	_tid = Spartan::thread_create((void*) &thread_start,
	                              (void *) (_context->stack_base),
	                              stack_size, _context->name);
	PWRN("%s: NEEDS FIX", __PRETTY_FUNCTION__);
}


void Thread_base::cancel_blocking()
{
	PWRN("%s: Not implemented, seems to be that Core doesn't need it",
	     __PRETTY_FUNCTION__);
}
