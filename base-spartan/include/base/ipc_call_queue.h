#ifndef _INCLUDE__BASE__IPC_CALL_QUEUE_H_
#define _INCLUDE__BASE__IPC_CALL_QUEUE_H_

#include <base/ipc_call.h>
#include <base/lock.h>

namespace Genode {
	enum {
		FULL_IPC_QUEUE = -20,
	};

	/* class to save incoming calls
	 * since it is not recommended to use dynamical lists (malloc)
	 * it has to be implemented in an array
	 *
	 * this class is ment to be used in a worker thread
	 */
	class Ipc_call_queue
	{
		enum {
			QUEUE_SIZE = 16,
		};

		private:
			Ipc_call	_call_queue[QUEUE_SIZE];
			bool		_used[QUEUE_SIZE];
			int		_call_count;
			Lock		_queue_lock;

			Ipc_call	*_call_list[QUEUE_SIZE];

			int		_get_first_free_slot();

		public:
			explicit Ipc_call_queue();

			int		call_count(void) { return _call_count; }
			int		max_call_count(void) { return QUEUE_SIZE; }
			int		insert_new(Ipc_call new_call);
			Ipc_call	get_first(addr_t imethod=0);
			Ipc_call	get_last(void);
	};
}

#endif /* _INCLUDE__BASE__IPC_CALL_QUEUE_H_ */
