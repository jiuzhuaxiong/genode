#ifndef _INCLUDE__BASE__IPC_CALL_H_
#define _INCLUDE__BASE__IPC_CALL_H_

#include <native_types.h>

namespace Genode {
	class Ipc_call
	{
		private:
			Native_ipc_call _call;

		public:
			explicit Ipc_call()
			: _call(Native_ipc_call(0, 0, 0, 0, 0, 0, 0, 0, 0));
			explicit Ipc_call(Native_ipc_call call)
			: _call(call) {}

			Native_ipc_callid callid() { return _call.callid; }
			Native_ipc_call call() { return _call; }


			addr_t method() { return IPC_GET_IMETHOD(_call); }
			addr_t arg1() { return IPC_GET_ARG1(_call); }
			addr_t arg2() { return IPC_GET_ARG2(_call); }
			addr_t arg3() { return IPC_GET_ARG3(_call); }
			addr_t arg4() { return IPC_GET_ARG4(_call); }
			addr_t arg5() { return IPC_GET_ARG5(_call); }

			Native_task snd_task_id()
			{
				return _call.in_task_id;
			}

			Native_thread_id snd_thread_id()
			{
				return arg3();
			}

			addr_t snd_phonehash()
			{
				return _call.in_phone_hash;
			}

			Native_thread_id dest_thread_id()
			{
				return arg4();
			}

			int cloned_phone()
			{
				switch(call_method()) {
				case IPC_M_CONNECTION_CLONE:
					return arg1();
				default:
					return -1;
				}
			}

			bool is_valid() {
				return (call.callid != 0);
			}

			bool operator == (Ipc_call other)
			{
				return ( (_callid == other.callid())
					&& (dest_thread_id() == other.dest_thread_id()));
			}

			bool operator != (Ipc_call other)
			{
				return !(this == other);
			}
	};
}

#endif /* _INCLUDE__BASE__IPC_CALL_H_*/

