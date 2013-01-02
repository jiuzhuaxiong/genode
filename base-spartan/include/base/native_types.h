/*
 * \brief  Spartan-specific queue preserving thread specific ipc calls
 * \author Tobias Börtitz
 * \date   2012-08-14
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__BASE__NATIVE_TYPES_H_
#define _INCLUDE__BASE__NATIVE_TYPES_H_

#include <base/native_capability.h>
#include <base/stdint.h>

namespace Spartan {
#include <abi/ipc/ipc.h>
}

namespace Genode {
	typedef volatile int    Native_lock;
	typedef          addr_t Native_thread_id;
	typedef          addr_t Native_thread;
	typedef          addr_t Native_task;

	typedef          addr_t Native_ipc_callid;
	/**
	 * This is a structure adopted from HelenOS/SPARTAN.
	 * It is important for ipc with SPARTAN's syscalls.
	 * Therefore the layout MUST be the same as in HelenOS/SPARTAN.
	 * Additions to the pre-defined values MUST be added at the very end.
	 */
	typedef struct {
		addr_t            args[IPC_CALL_LEN];
		Native_task       in_task_id;
		addr_t            in_phone_hash;
		/**
		 * Addition for compfortable handling of
		 *  ipc calls in Genode
		 */
		Native_ipc_callid callid;
	} Native_ipc_call;

	struct Ipc_destination {
		Native_thread_id rcv_thread_id;
		int              snd_phone;
	};

	struct Cap_dst_policy
	{
		typedef Ipc_destination Dst;
		static bool valid(Dst tid)
		{
			if((tid.rcv_thread_id == 0)
			   || (tid.snd_phone < 0))
				return false;
			return true;
		}
		static Dst  invalid()
		{
			Dst dest;
			dest.rcv_thread_id = 0;
			dest.snd_phone = -1;
			return dest;
		}
		static void copy(void* dst, Native_capability_tpl<Cap_dst_policy>* src);
	};

	typedef Native_capability_tpl<Cap_dst_policy> Native_capability;
	typedef addr_t Native_connection_state;

	struct Native_config
	{
		/**
		 * Thread-context area configuration.
		 */
		static addr_t context_area_virtual_base() { return 0x40000000UL; }
		static addr_t context_area_virtual_size() { return 0x10000000UL; }

		/**
		 * Size of virtual address region holding the context of one thread
		 */
		static addr_t context_virtual_size() { return 0x00100000UL; }
	};
}


#endif /* _INCLUDE__BASE__NATIVE_TYPES_H_ */
