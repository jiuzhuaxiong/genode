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
			Native_task       _task_id;
			Native_thread_id  _thread_id;
			Ipc_message_queue _msg_queue;

			bool             _waiting_for_ipc;

		public:
			explicit Thread_utcb()
			: _task_id(Spartan::task_get_id()),
			  _waiting_for_ipc(false) { }
			~Thread_utcb();

			Native_task      task_id() { return _task_id; }
			Native_thread_id thread_id() { return _thread_id; }
			void             set_thread_id(Native_thread_id tid);

			void             insert_msg(Ipc_message msg);
			Ipc_message      wait_for_call(addr_t imethod=0);
			Ipc_message      wait_for_answer(Native_ipc_callid callid=0);

			bool             is_waiting_for_ipc();
	};

	typedef Thread_utcb Native_utcb;
}

#endif /* _INCLUDE__BASE__THREAD_UTCB_H_ */
