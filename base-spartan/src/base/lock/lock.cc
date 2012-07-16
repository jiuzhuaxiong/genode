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
#include <cpu/atomic.h>
#include <base/printf.h>

/* Spartan includes */
#include <spartan/syscalls.h>

using namespace Genode;


Cancelable_lock::Cancelable_lock(Cancelable_lock::State initial)
: _native_lock(UNLOCKED), _locked_thread_count(0)
{
	if (initial == LOCKED)
		lock();
}


void Cancelable_lock::lock()
{
	while (!Genode::cmpxchg(&_native_lock, UNLOCKED, LOCKED)) {
		/* increase number of waiting threads, 
		 * to dertermine whether there are waiting threads to 
		 * be woken up when the lock is unlocked */
		_locked_thread_count++;
		if (Spartan::futex_sleep(&_native_lock) == -1) {
			_locked_thread_count--;
			throw Genode::Blocking_canceled();
		}
		_locked_thread_count--;
	}
}


void Cancelable_lock::unlock()
{
	_native_lock = UNLOCKED;

	/* wake up 1 sleeping thread when there is one waiting for the lock */
	if(_locked_thread_count)
		Spartan::futex_wakeup(&_native_lock);
}
