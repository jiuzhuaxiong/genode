#ifndef _IPC_CALL_H_
#define _IPC_CALL_H_

#include <spartan/syscalls.h>

namespace Genode {
	class Ipc_call
	{
		private:
			Native_ipc_callid	_callid;
			Native_ipc_call		_call;

		public:
			explicit Ipc_call()
			: _callid(0) {}
			explicit Ipc_call(Native_ipc_callid callid,
					Native_ipc_call call)
			: _callid(callid), _call(call) {}

			Native_ipc_callid callid() { return _callid; }
			Native_ipc_call call() { return _call; }


			addr_t		call_method() { return IPC_GET_IMETHOD(_call); }
			addr_t		call_arg1() { return IPC_GET_ARG1(_call); }
			addr_t		call_arg2() { return IPC_GET_ARG2(_call); }
			addr_t		call_arg3() { return IPC_GET_ARG3(_call); }
			addr_t		call_arg4() { return IPC_GET_ARG4(_call); }
			addr_t		call_arg5() { return IPC_GET_ARG5(_call); }

			Native_task 		snd_task_id() { return _call.in_task_id; }
			Native_thread_id 	snd_thread_id()
						{
							switch(call_method()) {
							case IPC_M_DATA_WRITE:
								return call_arg4();
							default:
								return call_arg1();
							}
						}
			addr_t			snd_phonehash() { return _call.in_phone_hash; }
			Native_thread_id	dest_thread_id()
						{
							switch(call_method()) {
							case IPC_M_DATA_WRITE:
								return call_arg3();
							default:
								return call_arg2();
							}
						}

			int		cloned_phone()
					{
						switch(call_method()) {
						case IPC_M_CONNECTION_CLONE:
							return call_arg1();
						default:
							return -1;
						}
					}
			Native_thread_id	target_thread_id()
						{
							switch(call_method()) {
							case IPC_M_CONNECTION_CLONE:
								return call_arg3();
							default:
								return Spartan::INVALID_ID;
							}
						}
			long		capability_id()
					{
						switch(call_method()) {
						case IPC_M_CONNECTION_CLONE:
							return call_arg4();
						default:
							return -1;
						}
					}
			addr_t		msg_size()
					{
						switch(call_method()) {
						case IPC_M_DATA_WRITE:
							return call_arg2();
						default:
							return Spartan::INVALID_ID;
						}
					}

			bool operator == (Ipc_call other)
			{
				return ( (_callid == other.callid())
						&& (dest_thread_id() == other.dest_thread_id()));
			}
			bool operator != (Ipc_call other) { return !(*this == other); }
	};
}

#endif /* _IPC_CALL_H_*/

