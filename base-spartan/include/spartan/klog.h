/*
 * \brief  Spartan klog syscalls
 * \author Tobias Börtitz
 * \date   2012-04-09
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__SPARTAN__KLOG_H_
#define _INCLUDE__SPARTAN__KLOG_H_

/* Genode includes */
#include <base/stdint.h>
#include <base/native_types.h>

/*Spartan includes */
#include <spartan/methods.h>

namespace Spartan
{
	Genode::addr_t klog_write(const void *buf, Genode::addr_t size);
//void klog_update(void);
	int klog_printf(const char *fmt, ...);
}

#endif /* _INCLUDE__SPARTAN__KLOG_H_ */
