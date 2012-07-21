/*
 * \brief  Lock implementation
 * \author Norman Feske
 * \date   2007-10-15
 */

/*
 * Copyright (C) 2007-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/cancelable_lock.h>
#include <base/printf.h>

/* Spartan includes */
#include <spartan/syscalls.h>

using namespace Genode;


Cancelable_lock::Cancelable_lock(Cancelable_lock::State initial)
: _native_lock(UNLOCKED)
{
	/* call wakeup so the first thread calling sleep
	 *  will return immediately */
	Spartan::futex_wakeup(&_native_lock);

	if (initial == LOCKED)
		lock();
}


void Cancelable_lock::lock()
{
	if (Spartan::futex_sleep(&_native_lock) == -1)
		throw Genode::Blocking_canceled();

	/* when returning from sleeping state set lock variable to locked
	 * using cmpxchg is not needed since this is happening when
	 *  the lock is already aquired */
	_native_lock = LOCKED;
}


void Cancelable_lock::unlock()
{
	_native_lock = UNLOCKED;

	/* wake up 1 sleeping thread */
	Spartan::futex_wakeup(&_native_lock);
}
