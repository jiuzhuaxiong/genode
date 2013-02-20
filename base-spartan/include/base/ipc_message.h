#ifndef _INCLUDE__BASE__IPC_CALL_H_
#define _INCLUDE__BASE__IPC_CALL_H_

/* Genode includes */
#include <base/native_types.h>

/* SPARTAN includes */
#include <spartan/ipc.h>


namespace Genode {
	class Ipc_message
	{
		private:
			Native_ipc_call _call;

			static Native_ipc_call _invalid_ipc_call()
			{
				Native_ipc_call ipc_call;
				for(int i=0; i<IPC_CALL_LEN; i++)
					ipc_call.args[i] = 0;
				ipc_call.in_task_id = 0;
				ipc_call.in_phone_hash = 0;
				ipc_call.callid = 0;

				return ipc_call;
			}

		public:
			explicit Ipc_message()
			: _call(_invalid_ipc_call()) {}

			explicit Ipc_message(Native_ipc_call call)
			: _call(call) {}

			Native_ipc_callid callid() { return _call.callid; }
			Native_ipc_call call() { return _call; }


			addr_t method() { return IPC_GET_IMETHOD(_call); }
			addr_t arg1() { return IPC_GET_ARG1(_call); }
			addr_t arg2() { return IPC_GET_ARG2(_call); }
			addr_t arg3() { return IPC_GET_ARG3(_call); }
			addr_t arg4() { return IPC_GET_ARG4(_call); }
			addr_t arg5() { return IPC_GET_ARG5(_call); }

			/******************
			 * returning id's *
			 ******************/

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

			Native_thread_id dst_thread_id()
			{
				return arg4();
			}

			/************************
			 * handle phone cloning *
			 ************************/

			Native_thread_id target_thread_id()
			{
				if(method() == IPC_M_CONNECTION_CLONE)
					return arg2();
				else
					return 0;
			}

			addr_t capability_id()
			{
				if(method() == IPC_M_CONNECTION_CLONE)
					return arg5();
				else
					return 0;
			}

			bool valid_data_write(addr_t rep_callid)
			{
				if(method() == IPC_M_DATA_WRITE)
					return (rep_callid == 0
						|| arg5() == rep_callid);
				else
					return true;
			}


			/********************
			 * handling answers *
			 ********************/

			bool is_answer() {
				return (callid() & IPC_CALLID_ANSWERED);
			}

			bool is_answer_to(Native_ipc_callid cmp_id) {
				return (is_answer()
				        && ((callid() & ~IPC_CALLID_ANSWERED)
				            == cmp_id));
			}

			addr_t answer_code() {
				if(!is_answer())
					return 0;

				return method();
			}


			int cloned_phone()
			{
				if(is_answer()
				   && (arg1() == IPC_M_PHONE_HANDLE))
					return arg5();
				else if(method() == IPC_M_CONNECTION_CLONE)
					return arg1();
				else
					return -1;
			}

			addr_t msg_size() {
				switch(method()) {
				case IPC_M_DATA_WRITE:
				case IPC_M_DATA_READ:
					return arg2();
				default:
					return 0;
				}
			}

			/***********************
			 * comparing operators *
			 ***********************/
			bool operator == (Ipc_message &other)
			{
				return ( (callid() == other.callid())
					&& (dst_thread_id() == other.dst_thread_id()));
			}

			bool operator != (Ipc_message &other)
			{
				return !(*this == other);
			}

			bool is_valid() {
				static Ipc_message cmp_msg;
				return (*this != cmp_msg);
			}
	};
}

#endif /* _INCLUDE__BASE__IPC_CALL_H_*/

