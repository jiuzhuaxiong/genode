#ifndef _INCLUDE__BASE__CAPABILITY_H_
#define _INCLUDE__BASE__CAPABILITY_H_

#include <util/string.h>
//#include <base/rpc.h>
#include <base/native_types.h>

namespace Genode {

	/**
	 * Capability that is not associated with a specific RPC interface
	 */
	typedef Native_capability Untyped_capability;


	template <typename RPC_INTERFACE>
	class Capability
	{
		public: bool valid() const { return false; }
	};
	typedef int Connection_state;

	/**
	 * Convert an untyped capability to a typed capability
	 */
	template <typename RPC_INTERFACE>
	Capability<RPC_INTERFACE>
	reinterpret_cap_cast(Untyped_capability const &untyped_cap)
	{
		Capability<RPC_INTERFACE> typed_cap;

		/*
		 * The object layout of untyped and typed capabilities is identical.
		 * Hence we can use memcpy to load the values of the supplied untyped
		 * capability into a typed capability.
		 */
		::Genode::memcpy(&typed_cap, &untyped_cap, sizeof(untyped_cap));
		return typed_cap;
	}
}


#endif /* _INCLUDE__BASE__CAPABILITY_H_ */
