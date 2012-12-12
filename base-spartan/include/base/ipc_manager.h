#ifndef _MANGER_THREAD_H_
#define _MANGER_THREAD_H_

#include <base/native_types.h>
#include <base/lock.h>
#include <base/thread.h>

namespace Genode {
	/* call implementing the manager thread which is exclusively looking into
	 * the tasks answerbox, waiting for incoming calls and saving them.
	 * Other threads obtain calls from this class
	 * There is a maximum of one single Ipc_manager_thread per task
	 */
	class Ipc_manager
	{
		enum {
			/* governot states */
			GOV_FREE,
			GOV_TAKEN,

			/* maximum number of threads to serve */
			MAX_THREAD_COUNT = 16,
		};

		private:
			int          _governor;
			Native_utcb* _threads[MAX_THREAD_COUNT];
			addr_t       _thread_count;
			Lock         _thread_lock;

			explicit     Ipc_manager()
			: _thread_id(Spartan::INVALID_ID), _initialized(false) {}

			bool         _create();
			int          _get_thread(Native_thread_id thread_id);
			int          _get_thread(Thread_utcb* utcb);

		public:
			static Ipc_manager* singleton()
			{
				static Ipc_manager manager;
				return &manager;
			}

			void                get_call();
			bool                register_thread(Thread_utcb* utcb);
			void                unregister_thread(Thread_utcb *utcb);
	};
}

#endif /* _MANGER_THREAD_H_ */
