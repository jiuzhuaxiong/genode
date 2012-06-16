#ifndef _INCLUDE__BASE__NATIVE_TYPES_H_
#define _INCLUDE__BASE__NATIVE_TYPES_H_

#include <base/native_capability.h>
#include <base/stdint.h>

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

	typedef          addr_t Native_ipc_callid;
	typedef struct {
		addr_t args[IPC_CALL_LEN];
		Native_thread_id in_task_id;
		addr_t in_phone_hash;
	} Native_ipc_call;

	struct Cap_dst_policy
	{
		/* Destination - equals a phone*/
		typedef int Dst;
		static bool valid(Dst tid) { return false; }
		static Dst  invalid()      { return -1; }
		static void copy(void* dst, Native_capability_tpl<Cap_dst_policy>* src);
	};

	typedef Native_capability_tpl<Cap_dst_policy> Native_capability;
	typedef int Native_connection_state;
}

namespace Spartan {
	enum {
		INVALID_THREAD_ID = ~0UL,
	};

}

#endif /* _INCLUDE__BASE__NATIVE_TYPES_H_ */
