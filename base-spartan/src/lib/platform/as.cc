/*
 * \brief  Spartan-specific syscall implemetation
 * \author Tobias Börtitz
 * \date   2012-03-15
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <util/string.h>
#include <base/printf.h>

/* Spartan includes */
#include <spartan/syscalls.h>
#include <spartan/as.h>

using namespace Genode;

extern "C" {
#include <abi/ipc/methods.h>
}

extern "C" int _main();

using namespace Spartan;

void* Spartan::as_area_create(void *base, Genode::addr_t size,
                              unsigned int flags)
{
	return (void *) __SYSCALL4(SYS_AS_AREA_CREATE, (addr_t) base,
	                           (addr_t) size, (addr_t) flags,
	                           (addr_t) _main);
}

int Spartan::as_area_resize(void *address, Genode::addr_t size,
                            unsigned int flags)
{
	return __SYSCALL3(SYS_AS_AREA_RESIZE, (addr_t) address,
	                  (addr_t) size, (addr_t) flags);
}

int Spartan::as_area_destroy(void *address)
{
	return __SYSCALL1(SYS_AS_AREA_DESTROY, (addr_t) address);
}

int Spartan::as_area_change_flags(void *address, unsigned int flags)
{
	return __SYSCALL2(SYS_AS_AREA_CHANGE_FLAGS, (addr_t) address,
	                  (addr_t) flags);
}

