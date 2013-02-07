#ifndef _INCLUDE__BASE__THREAD_UTCB_H_
#define _INCLUDE__BASE__THREAD_UTCB_H_

/* Genode includes */
#include <base/native_types.h>
#include <base/ipc_message.h>
#include <base/ipc_message_queue.h>
#include <base/lock.h>

/* SPARTAN includes */
#include <spartan/syscalls.h>


namespace Genode {
	class Thread_utcb
	{
		private:
			Native_thread_id  _thread_id;
			Native_thread_id  _global_thread_id;
			Ipc_message_queue _msg_queue;

			bool             _waiting_for_ipc;

			class Thread_counter
			{
				private:
					addr_t _cnt;
				public:
					Thread_counter()
					: _cnt(0) {}

					addr_t inc() {
						return ++_cnt;
					}
			};

			Thread_counter*  _thread_counter() {
				static Thread_counter counter;
				return &counter;
			}

		public:
			explicit Thread_utcb();
//			: _task_id(Spartan::task_get_id()),
//			  _waiting_for_ipc(false),
//			  _thread_count(0);
			~Thread_utcb();

			Native_thread_id thread_id() { return _thread_id; }
			Native_thread_id global_thread_id() { return _global_thread_id; }
			void             set_thread_id(bool is_main_thread = 0);

			void             insert_msg(Ipc_message msg);
			Ipc_message      wait_for_call(addr_t imethod=0);
			Ipc_message      wait_for_answer(Native_ipc_callid callid=0);

			bool             is_waiting_for_ipc();

	};

	typedef Thread_utcb Native_utcb;
}

#endif /* _INCLUDE__BASE__THREAD_UTCB_H_ */
