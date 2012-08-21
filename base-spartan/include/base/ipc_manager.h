/*
 * \brief  Spartan-specific dispatcher class managing incomming ipc calls
 * \author Tobias Börtitz
 * \date   2012-08-14
 *
 * This class is ment to have exclusive access to the task specific
 *  answerbox. To be able to deliver the incoming calls to their
 *  destined thread, the thread has to register it's UTCB with the
 *  manager. Incoming calls are automatically stored in the thread
 *  specific call queue, located in the UTCB. Incoming calls which
 *  can not be delivered are answered with an errorcode.
 * As soon as the first worker thread is waiting for incomming calls
 *  the manager thread is beeing started.
 * One task may only have one Ipc_manager thread.
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _MANGER_THREAD_H_
#define _MANGER_THREAD_H_

/* Genode includes */
#include <base/native_types.h>
#include <base/lock.h>
#include <base/thread.h>

namespace Genode {
	class Ipc_manager
	{
		enum {
			STACK_SIZE = 8192,
			MAX_THREAD_COUNT = 16,
		};

		private:
			Native_thread_id _thread_id;
			bool             _initialized;
			Native_utcb*     _threads[MAX_THREAD_COUNT];
			addr_t           _thread_count;
			Lock             _thread_lock;

			explicit Ipc_manager()
			: _thread_id(Spartan::INVALID_ID), _initialized(false) {}

			bool _create();
			int  _get_thread(Native_thread_id thread_id);
			int  _get_thread(Thread_utcb* utcb);

		public:
			static Ipc_manager* singleton()
			{
				static Ipc_manager manager;
				return &manager;
			}
			void wait_for_calls();
			void loop_answerbox();
			bool register_thread(Thread_utcb* utcb);
			void unregister_thread(Thread_utcb *utcb);
	};
}

#endif /* _MANGER_THREAD_H_ */
