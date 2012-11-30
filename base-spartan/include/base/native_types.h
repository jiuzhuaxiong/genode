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
	enum {
		INVALID_ID = ~0UL,
	};
}

namespace Genode {
	/* TODO nasty work around
	 * declaration taken from helenos/abi/include/ipc.ipc.h
	 * It represents the maximum count of argument that can be passed 
	 *  with a ipc call
	 */
	enum {
		IPC_CALL_LEN = 6,
	};

	typedef volatile int Native_lock;
	typedef          addr_t Native_thread_id;
	typedef          addr_t Native_thread;
	typedef          addr_t Native_task;

	/**
	 * This is a structure adopted from HelenOS/SPARTAN.
	 * It is important for ipc with SPARTAN's syscalls.
	 * Therefore the layout MUST be the same as in HelenOS/SPARTAN.
	 * Additions to the pre-defined values MUST be added at the very end.
	 */
	typedef struct {
		addr_t      args[IPC_CALL_LEN];
		Native_task in_task_id;
		addr_t      in_phone_hash;
		/**
		 * Addition for compfortable handling of
		 *  ipc calls in Genode
		 */
		addr_t      callid;
	} Native_ipc_call;


	struct Ipc_destination {
		Native_thread_id rcv_thread_id;
		int              snd_phone;
	};

	struct Cap_dst_policy
	{
		typedef Ipc_destination Dst;
		static bool valid(Dst tid) { return false; }
		static Dst  invalid()
		{
			Dst dest;
			dest.rcv_thread_id = Spartan::INVALID_ID;
			dest.snd_phone = 0;
			return dest;
		}
		static void copy(void* dst, Native_capability_tpl<Cap_dst_policy>* src);
	};

	typedef Native_capability_tpl<Cap_dst_policy> Native_capability;
	typedef addr_t Native_connection_state;

	// temporary workaround until i need a real Native_utcb
	typedef struct { } Native_utcb;
}


#endif /* _INCLUDE__BASE__NATIVE_TYPES_H_ */
