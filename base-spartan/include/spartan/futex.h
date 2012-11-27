/*
 * \brief  Spartan futex specific syscalls
 * \author Tobias Börtitz
 * \date   2012-11-27
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__SPARTAN__FUTEX_H_
#define _INCLUDE__SPARTAN__FUTEX_H_


#include <base/stdint.h>

namespace Spartan
{

	/**
	 * Wakes up one sleeping thread.
	 * If there is no sleeping thread the next call to futex_sleep()
	 *  will return immediately.
	 */
	int futex_wakeup(volatile int *futex);

	/**
	 * Sends a thread into sleep.
	 * Will return immediately as many times as futex_wakeup() was called
	 *  when there where no sleeping threads befor sending the first thread into
	 *  sleep.
	 */
	int futex_sleep(volatile int *futex);
}

#endif /* _INCLUDE__SPARTAN__FUTEX_H_ */
