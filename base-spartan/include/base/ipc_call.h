/*
 * \brief  Spartan-specific class representing a single ipc call
 * \author Tobias Börtitz
 * \date   2012-08-14
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__BASE__IPC_CALL_H_
#define _INCLUDE__BASE__IPC_CALL_H_

/* spartan includes */
#include <spartan/syscalls.h>


namespace Genode {

	class Ipc_call
	{
		private:
			Native_ipc_callid _callid;
			Native_ipc_call   _native_call;

		public:
			explicit Ipc_call()
			: _callid(0) {}
			explicit Ipc_call(Native_ipc_callid callid,
					Native_ipc_call call)
			: _callid(callid), _native_call(call) {}

			Native_ipc_callid callid() { return _callid; }
			Native_ipc_call call() { return _native_call; }


			addr_t call_method() { return IPC_GET_IMETHOD(_native_call); }
			addr_t call_arg1() { return IPC_GET_ARG1(_native_call); }
			addr_t call_arg2() { return IPC_GET_ARG2(_native_call); }
			addr_t call_arg3() { return IPC_GET_ARG3(_native_call); }
			addr_t call_arg4() { return IPC_GET_ARG4(_native_call); }
			addr_t call_arg5() { return IPC_GET_ARG5(_native_call); }

			Native_task snd_task_id() { return _native_call.in_task_id; }

			Native_thread_id snd_thread_id() {
				switch(call_method()) {
				case IPC_M_DATA_WRITE:
				case IPC_M_DATA_READ:
					return call_arg4();
				default:
					return call_arg1();
				}
			}

			addr_t snd_phonehash() {
			         return _native_call.in_phone_hash; }

			Native_thread_id dest_thread_id() {
				switch(call_method()) {
				case IPC_M_DATA_WRITE:
				case IPC_M_DATA_READ:
					return call_arg3();
				default:
					return call_arg2();
				}
			}

			int cloned_phone() {
				switch(call_method()) {
				case IPC_M_CONNECTION_CLONE:
					return call_arg1();
				default:
					return -1;
				}
			}

			Native_thread_id target_thread_id() {
				switch(call_method()) {
				case IPC_M_CONNECTION_CLONE:
					return call_arg3();
				default:
					return Spartan::INVALID_ID;
				}
			}

			long capability_id() {
				switch(call_method()) {
				case IPC_M_CONNECTION_CLONE:
				return call_arg4();
				default:
					return -1;
				}
			}
			addr_t msg_size() {
				switch(call_method()) {
				case IPC_M_DATA_WRITE:
				case IPC_M_DATA_READ:
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

#endif /* _INCLUDE__BASE__IPC_CALL_H_ */

