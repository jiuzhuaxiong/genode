/*
 * \brief  Spartan syscalls
 * \author Tobias Börtitz
 * \date   2012-08-14
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__SPARTAN__SYSCALLS_H_
#define _INCLUDE__SPARTAN__SYSCALLS_H_

#include <base/stdint.h>
#include <base/native_types.h>

namespace Spartan
{
#include <sys/types.h>
#include <abi/ipc/ipc.h>
#include <abi/ipc/methods.h>
#include <abi/mm/as.h>
#include <abi/errno.h>

	/*************************
	 * Basic functionalities *
	 ************************/

	void exit(int status);

	void io_port_enable(Genode::addr_t pio_addr, Genode::size_t size);


	/***************************
	 * Access to specific ID's *
	 **************************/
	Genode::Native_task task_get_id(void);
	Genode::Native_thread_id thread_get_id(void);
	Genode::Native_thread thread_create(void* ip, void* sp, 
			Genode::addr_t stack_size, const char* name);
}

#endif /* _INCLUDE__SPARTAN__SYSCALLS_H_ */
