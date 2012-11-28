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
//#include <abi/ipc/ipc.h>
//#include <abi/ipc/methods.h>
//#include <abi/mm/as.h>
#include <abi/errno.h>
#include <abi/syscall.h>

	/*************************************
	 * Syscall and its wrapper functions *
	 *************************************/

	extern "C" Genode::addr_t __syscall(const Genode::addr_t p1,
	                                    const Genode::addr_t p2,
	                                    const Genode::addr_t p3,
	                                    const Genode::addr_t p4,
	                                    const Genode::addr_t p5,
	                                    const Genode::addr_t p6,
	                                    const syscall_t id);

	inline Genode::addr_t  __SYSCALL0(syscall_t id) {
		return __syscall(0, 0, 0, 0, 0, 0, id);
	}

	inline Genode::addr_t  __SYSCALL1(syscall_t id, Genode::addr_t p1) {
		return __syscall(p1, 0, 0, 0, 0, 0, id);

	}

	inline Genode::addr_t  __SYSCALL2(syscall_t id, Genode::addr_t p1,
	                                  Genode::addr_t p2) {
		return __syscall(p1, p2, 0, 0, 0, 0, id);
	}

	inline Genode::addr_t  __SYSCALL3(syscall_t id, Genode::addr_t p1,
	                                  Genode::addr_t p2, Genode::addr_t p3) {
		return __syscall(p1, p2, p3, 0, 0, 0, id);
	}

	inline Genode::addr_t  __SYSCALL4(syscall_t id, Genode::addr_t p1,
	                                  Genode::addr_t p2, Genode::addr_t p3,
	                                  Genode::addr_t p4) {
		return __syscall(p1, p2, p3, p4, 0, 0, id);
	}

	inline Genode::addr_t  __SYSCALL5(syscall_t id, Genode::addr_t p1,
	                                  Genode::addr_t p2, Genode::addr_t p3,
					  Genode::addr_t p4, Genode::addr_t p5) {
		return __syscall(p1, p2, p3, p4, p5, 0, id);
	}

	inline Genode::addr_t  __SYSCALL6(syscall_t id, Genode::addr_t p1,
	                                  Genode::addr_t p2, Genode::addr_t p3,
	                                  Genode::addr_t p4, Genode::addr_t p5,
	                                  Genode::addr_t p6) {
		return __syscall(p1, p2, p3, p4, p5, p6, id);
	}

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
	                                    Genode::addr_t stack_size,
	                                    const char* name);

}

#endif /* _INCLUDE__SPARTAN__SYSCALLS_H_ */
