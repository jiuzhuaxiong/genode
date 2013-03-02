#ifndef _INCLUDE__BASE__THREAD_UTCB_H_
#define _INCLUDE__BASE__THREAD_UTCB_H_

/* Genode includes */
#include <base/native_types.h>
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
					explicit Thread_counter()
					: _cnt(0) {}

					addr_t new_id() {
						return ++_cnt;
					}
			};

			addr_t _new_thread_id() {
				static Thread_counter counter;
				return counter.new_id();
			}

		public:
			explicit Thread_utcb();
			~Thread_utcb();

			Native_thread_id   thread_id() { return _thread_id; }
			Native_thread_id   global_thread_id() { return _global_thread_id; }

			Ipc_message_queue* msg_queue() { return &_msg_queue; }

			bool             is_waiting_for_ipc();

	};

	typedef Thread_utcb Native_utcb;
}

#endif /* _INCLUDE__BASE__THREAD_UTCB_H_ */