/*
 * \brief  Spartan syscalls for addess space handling
 * \author Tobias Börtitz
 * \date   2013-03-15
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__SPARTAN__AS_H_
#define _INCLUDE__SPARTAN__AS_H_

/* Genode includes */
#include <base/stdint.h>
#include <base/native_types.h>

/*Spartan includes */
#include <spartan/methods.h>

namespace Spartan
{
#include <sys/types.h>
#include <abi/mm/as.h>

	void *as_area_create(void *base, Genode::addr_t size,
	                     unsigned int flags);

	int as_area_resize(void *address, Genode::addr_t size,
	                   unsigned int flags);

	int as_area_destroy(void *address);

	int as_area_change_flags(void *address, unsigned int flags);
}

#endif /* _INCLUDE__SPARTAN__AS_H_ */
