#ifndef _INCLUDE__BASE__THREAD_UTCB_H_
#define _INCLUDE__BASE__THREAD_UTCB_H_

/* Genode includes */
#include <base/native_types.h>
#include <base/ipc_call.h>
#include <base/ipc_call_queue.h>
#include <base/lock.h>

/* SPARTAN includes */
#include <spartan/syscalls.h>


namespace Genode {
	class Thread_utcb
	{
		private:
			Native_task      _task_id;
			Native_thread_id _thread_id;
			Ipc_call_queue   _call_queue;

		public:
			explicit Thread_utcb()
			: _task_id(Spartan::task_get_id()) { }
			~Thread_utcb();

			Native_task      task_id() { return _task_id; }
			Native_thread_id thread_id() { return _thread_id; }
			void             set_thread_id(Native_thread_id tid);

			void             insert_call(Ipc_call call);
			Ipc_call         wait_for_call(addr_t imethod=0);
			Ipc_call         wait_for_reply(Native_ipc_callid callid=0);

			bool             is_waiting();
	};

	typedef Thread_utcb Native_utcb;
}

#endif /* _INCLUDE__BASE__THREAD_UTCB_H_ */
