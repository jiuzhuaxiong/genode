/*
 * \brief  Spartan-specific klog implementation writing to SPARTAN's kernel console
 * \author Tobias Börtitz
 * \date   2012-04-09
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/snprintf.h>

/* Spartan includes */
#include <spartan/syscalls.h>
#include <spartan/klog.h>

using namespace Genode;

using namespace Spartan;

addr_t Spartan::klog_write(const void *buf, addr_t size)
{
	return __SYSCALL3(SYS_KLOG, 1, (addr_t) buf, size);
}

int Spartan::klog_printf(const char *fmt, ...)
{
	va_list list;
	va_start(list, fmt);

	char   buffer[100];

	String_console sc(buffer, 100);
	sc.vprintf(fmt, list);

	klog_write(buffer, sc.len());

	va_end(list);
	return sc.len();
}

