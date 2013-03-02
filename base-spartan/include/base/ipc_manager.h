#ifndef _MANGER_THREAD_H_
#define _MANGER_THREAD_H_

#include <base/native_types.h>
#include <base/lock.h>
#include <base/thread_utcb.h>

namespace Genode {

	template <int QUEUE_SIZE>
	class Thread_buffer
	{
		private:
			Lock         _thread_lock;
			Native_utcb* _threads[QUEUE_SIZE];
			int          _thread_count;

		public:
			class Overflow : public Genode::Exception { };

			Thread_buffer() : _thread_count(0) {};
			void         add(Native_utcb* utcb);
			/**
			 * looks up the list whether the request thread_utcb
			 *  exists or not
			 * returns position relative to _tail when found
			 * returns 0 when not found
			 */
			Native_utcb* exists_threadid(Native_thread_id thread_id);
			Native_utcb* exists_global_threadid(Native_thread_id thread_id);
			int          exists_utcbpt(Thread_utcb* utcb);

			void         message_first_waiting(Ipc_message msg,
			                                   Native_thread_id thread_id);
			void         del(Thread_utcb* utcb);
	};

	/* call implementing the manager thread which is exclusively looking into
	 * the tasks answerbox, waiting for incoming calls and saving them.
	 * Other threads obtain calls from this class
	 * There is a maximum of one single Ipc_manager_thread per task
	 */
	class Ipc_manager
	{
		enum {
			/* governot states */
			GOV_FREE = 0,

			/* maximum number of threads to serve */
			MAX_THREAD_COUNT = 16,
		};

		private:
			addr_t                          _governor;

			Thread_buffer<MAX_THREAD_COUNT> _threads;
			Lock                            _thread_lock;

			explicit     Ipc_manager()
			: _governor(GOV_FREE) {}

			void         _wait_for_calls();

		public:
			static Ipc_manager* singleton()
			{
				static Ipc_manager manager;
				return &manager;
			}

			bool                get_call(Native_thread_id thread_id);
			Native_utcb*        my_utcb();
			void                register_thread(Thread_utcb* utcb);
			void                unregister_thread(Thread_utcb *utcb);
	};
}

#endif /* _MANGER_THREAD_H_ */
