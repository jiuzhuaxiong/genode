#ifndef _INCLUDE__BASE__NATIVE_TYPES_H_
#define _INCLUDE__BASE__NATIVE_TYPES_H_

#include <base/native_capability.h>

namespace Genode {
	typedef volatile int Native_lock;
	typedef          int Native_thread_id;
	typedef          int Native_thread;

	struct Cap_dst_policy
	{
		typedef int Dst;
		static bool valid(Dst tid) { return false; }
		static Dst  invalid()      { return 0; }
		static void copy(void* dst, Native_capability_tpl<Cap_dst_policy>* src);
	};

	typedef Native_capability_tpl<Cap_dst_policy> Native_capability;
	typedef int Native_connection_state;
}

#endif /* _INCLUDE__BASE__NATIVE_TYPES_H_ */
