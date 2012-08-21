/*
 * \brief  Spartan-specific Native_utcb implementation
 * \author Tobias Börtitz
 * \date   2012-08-14
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__BASE__THREAD_UTCB_H_
#define _INCLUDE__BASE__THREAD_UTCB_H_

/* Genode includes */
#include <base/native_types.h>
#include <base/ipc_call.h>
#include <base/ipc_call_queue.h>
#include <base/lock.h>


namespace Genode {
	enum {
		THREAD_KILLED = -30,
	};

	class Thread_utcb
	{
		private:
			Native_task      _task_id;
			Native_thread_id _thread_id;
			Ipc_call_queue   _call_queue;

			Ipc_call         _ipc_answer;
			Semaphore        _answer_sem;
			Lock             _answer_lock;

		public:
			explicit Thread_utcb()
			: _task_id(Spartan::task_get_id()) { }
			~Thread_utcb();

			Native_task	task_id() { return _task_id; }
			Native_thread_id thread_id() { return _thread_id; }
			void		set_thread_id(Native_thread_id tid);
			void		insert_call(Ipc_call call);
			Ipc_call	get_next_call(addr_t imethod=0);
			Ipc_call	wait_for_call(addr_t imethod=0);

			bool		insert_reply(Ipc_call call);
			Ipc_call	get_reply();
	};

	typedef Thread_utcb Native_utcb;
}

#endif /* _INCLUDE__BASE__THREAD_UTCB_H_ */
