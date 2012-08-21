/*
 * \brief  Implementation of the core-internal Thread API via Spartan's threads
 * \author Norman Feske
 * \author Tobias Börtitz
 * \date   2012-08-14
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


static void thread_start(void *)
{
	Thread_base::myself()->entry();
	sleep_forever();
	PWRN("%s: Not implemented", __PRETTY_FUNCTION__);
}

void Thread_base::start()
{
	/**
	 * calculate used stack size, since Spartans syscall
	 * requires knowledge about this
	 */
	addr_t stack_size = (addr_t) _context->stack - (addr_t) _context->stack_base; //8192;//16384;

	/* create the thread */
	_tid = Spartan::thread_create((void*) &thread_start,
	                              (void *) (_context->stack_base),
	                              stack_size, _context->name);
	/* set the thread id in threads UTCB */
	utcb()->set_thread_id(_tid);
	PWRN("%s: needs fix?", __PRETTY_FUNCTION__);
}


void Thread_base::cancel_blocking()
{
	PWRN("%s: Not implemented, seems to be that Core doesn't need it",
	     __PRETTY_FUNCTION__);
}

