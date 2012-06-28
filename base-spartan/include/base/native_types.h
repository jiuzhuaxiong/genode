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

	typedef          addr_t Native_ipc_callid;
	typedef struct {
		addr_t args[IPC_CALL_LEN];
		Native_task in_task_id;
		addr_t in_phone_hash;
	} Native_ipc_call;
/*
	class Ipc_destination
	{
		private:
			Native_task	in_task_id;
			Native_thread	in_thread_id;
			Native_task	out_task_id;
			Native_thread	out_thread_id;
			int		out_phone;
			addr_t		out_phone_hash;
		public:
			Ipc_destination(Native_task r_task=0,
					Native_thread r_thread=0,
					Native_task s_task=0,
					Native_thread s_thread=0,
					int s_phone=0, addr_t s_phonehash=0)
			:
				in_task_id(r_task), in_thread_id(r_thread),
				out_task_id(s_task), out_thread_id(s_thread),
				out_phone(s_phone), out_phone_hash(s_phonehash)
			{}

			Native_task rcv_task_id() { return in_task_id; }
			Native_thread rcv_thread_id() { return in_thread_id; }
			Native_task snd_task_id() { return out_task_id; }
			Native_thread snd_thread_id() { return out_thread_id; }
			int snd_phone() { return out_phone; }
			addr_t snd_phone_hash() { return out_phone_hash; }
	};
*/
	struct Ipc_destination {
		Native_task		rcv_task_id;
		Native_thread_id	rcv_thread_id;
		Native_task		snd_task_id;
		Native_thread_id	snd_thread_id;
		int			snd_phone;
		addr_t			snd_phonehash;
	};

	struct Cap_dst_policy
	{
		typedef Ipc_destination Dst;
		static bool valid(Dst tid) { return false; }
//		static Dst  invalid()	{ return Dst(-1, -1, -1, -1, -1, -1); }
		static Dst  invalid()
		{
			Dst dest;
			dest.rcv_task_id = Spartan::INVALID_ID;
			dest.rcv_thread_id = Spartan::INVALID_ID;
			dest.snd_task_id = Spartan::INVALID_ID;
			dest.snd_thread_id = Spartan::INVALID_ID;
			dest.snd_phone = 0;
			dest.snd_phonehash = 0;
			return dest;
		}
		static void copy(void* dst, Native_capability_tpl<Cap_dst_policy>* src);
	};

	typedef Native_capability_tpl<Cap_dst_policy> Native_capability;
	typedef addr_t Native_connection_state;
}


#endif /* _INCLUDE__BASE__NATIVE_TYPES_H_ */
