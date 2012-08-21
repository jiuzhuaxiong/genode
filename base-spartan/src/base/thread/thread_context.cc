/*
 * \brief  Spartan-specific thread context implementation
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
#include <base/thread.h>
#include <base/thread_utcb.h>
#include <base/ipc_manager.h>
#include <base/printf.h>

/* Spartan includes */
#include <spartan/syscalls.h>

using namespace Genode;


Native_utcb *main_thread_utcb()
{
	static Native_utcb _main_utcb;
	return &_main_utcb;
}


void Thread_base::_init_platform_thread()
{
	/* not used because executed in old thread's context */
	PWRN("%s: Not implemented and not needed.", __PRETTY_FUNCTION__);
}


void Thread_base::_deinit_platform_thread()
{
	PWRN("%s: Not implemented and not needed.", __PRETTY_FUNCTION__);
}


Native_utcb *Thread_base::utcb()
{
	/*
	 * If 'utcb' is called on the object returned by 'myself',
	 * the 'this' pointer may be NULL (if the calling thread is
	 * the main thread). Therefore we allow this special case
	 * here.
	 */
	if (this == 0) return main_thread_utcb();

	return &_context->utcb;
}
