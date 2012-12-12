#ifndef _INCLUDE__BASE__IPC_CALL_QUEUE_H_
#define _INCLUDE__BASE__IPC_CALL_QUEUE_H_

#include <base/lock.h>
#include <base/semaphore.h>
#include <base/exception.h>

#include <base/ipc_call.h>

namespace Genode {
	enum {
		QUEUE_SIZE = 16,
	};

	/* class to save incoming calls
	 * since it is not recommended to use dynamical lists (malloc)
	 * it has to be implemented in an array
	 *
	 * this class is ment to be used in a worker thread
	 */
	class Ipc_call_queue
	{
		private:
			Ipc_call  _queue[QUEUE_SIZE];
			addr_t    _item_count;
			/* semaphore for counting not visited calls when
			 *  looking for a specific call
			 * has to be reset to the number of calls waiting
			 *  in the ring_buffer (_sem.cnt()) every time waiting
			 *  for a call is finished */
			Semaphore _sem;
			Lock      _read_lock, _write_lock;

		public:
			class Overflow : public Genode::Exception { };

			/**
			 * Constructor
			 */
			Ipc_call_queue()
			: _item_count(0) {}

			void		insert_new(Ipc_call new_call);
			Ipc_call	get_first(addr_t imethod=0);
			Ipc_call	get_last(void);
	};
}

#endif /* _INCLUDE__BASE__IPC_CALL_QUEUE_H_ */
