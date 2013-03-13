#ifndef _INCLUDE__BASE__IPC_CALL_QUEUE_H_
#define _INCLUDE__BASE__IPC_CALL_QUEUE_H_

#include <base/lock.h>
#include <base/semaphore.h>
#include <base/exception.h>

#include <base/ipc_message.h>

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
	class Ipc_message_queue
	{
		private:
			Ipc_message _queue[QUEUE_SIZE];
			addr_t      _item_count;
			addr_t      _msg_pt;
			/* semaphore for counting not visited calls when
			 *  looking for a specific call
			 * has to be reset to the number of calls waiting
			 *  in the ring_buffer (_sem.cnt()) every time waiting
			 *  for a call is finished */
			Semaphore   _sem;
			Lock        _read_lock, _write_lock;

			bool        (*_cmp_fktn)(Ipc_message, addr_t);
			addr_t      _cmp_val;

			void        _remove_from_queue(addr_t pos);
			Ipc_message _get_first(Native_thread_id thread_id);

		public:
			class Overflow : public Genode::Exception { };

			/**
			 * Constructor
			 */
			Ipc_message_queue()
			: _item_count(0), _msg_pt(0), _cmp_fktn(0) {}

			bool        insert(Ipc_message new_msg);
			Ipc_message wait_for_call(Native_thread_id thread_id,
			                          addr_t imethod=0,
			                          addr_t rep_callid=0);
			Ipc_message wait_for_answer(Native_thread_id thread_id,
			                            addr_t msgid=0);
			Ipc_message get_last(void);

			bool        is_waiting();
	};
}

#endif /* _INCLUDE__BASE__IPC_CALL_QUEUE_H_ */
