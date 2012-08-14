/*
 * \brief   Linux platform interface implementation
 * \author  Norman Feske
 * \date    2006-06-13
 */

/*
 * Copyright (C) 2006-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/lock.h>

/* local includes */
#include "platform.h"
#include "core_env.h"

using namespace Genode;


static char _some_mem[80*1024];
static Lock _wait_for_exit_lock(Lock::LOCKED);  /* exit() sync */


Platform::Platform()
	: _ram_alloc(0)
{
	addr_t tid_nameserv;
	/**
	 * TODO:
	 * - register with dummy nameserv
	 * - gain phone handly to myself
	 */
	Spartan::ipc_connect_to_me(PHONE_TO_NAMESERV, Spartan::thread_get_id(),
	                           0, 0, &tid_nameserv, &_phonehash_nameserv);

	_ram_alloc.add_range((addr_t)_some_mem, sizeof(_some_mem));
}


void Platform::wait_for_exit()
{
	/* block until exit condition is satisfied */
	try { _wait_for_exit_lock.lock(); }
	catch (Blocking_canceled) { };
}

void Core_parent::exit(int exit_value)
{
}

