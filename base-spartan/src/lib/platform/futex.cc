/*
 * \brief  Spartan-specific syscall implemetation of futex calls
 * \author Tobias Börtitz
 * \date   2012-11-27
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Spartan includes */
#include <spartan/syscalls.h>
#include <spartan/futex.h>

using namespace Genode;

extern "C" {
#include <abi/syscall.h>
}

using namespace Spartan;

int Spartan::futex_sleep(volatile int *futex)
{
	return __SYSCALL1(SYS_FUTEX_SLEEP, (sysarg_t) futex);
}


int Spartan::futex_wakeup(volatile int *futex)
{
	return __SYSCALL1(SYS_FUTEX_WAKEUP, (sysarg_t) futex);
}

