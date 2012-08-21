/*
 * \brief  Spartan-specific queue preserving thread specific ipc calls
 * \author Tobias Börtitz
 * \date   2012-08-14
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__BASE__IPC_CALL_QUEUE_H_
#define _INCLUDE__BASE__IPC_CALL_QUEUE_H_

/* Genode includes */
#include <base/lock.h>
#include <base/semaphore.h>
#include <base/exception.h>
#include <os/ring_buffer.h>

/* spartan includes */
#include <base/ipc_call.h>

namespace Genode {

	enum {
		FULL_IPC_QUEUE = -20,
		QUEUE_SIZE = 16,
	};

	class Ipc_call_queue : private Ring_buffer<Ipc_call, QUEUE_SIZE>
	{
		private:
			/**
			 * semaphore for counting not visited calls when
			 *  looking for a specific call
			 * has to be reset to the number of calls waiting
			 *  in the ring_buffer (_sem.cnt()) every time when
			 *  waiting for a call is finished
			 */
			Semaphore _unchecked_sem;

			Lock      _read_lock;
			Lock      _write_lock;

		public:
			class Overflow : public Genode::Exception { };

			/**
			 * Constructor
			 */
			Ipc_call_queue()
			: Ring_buffer() {}

			void     insert_new(Ipc_call new_call);
			Ipc_call get_first(addr_t imethod=0);
			Ipc_call get_last(void);
	};
}

#endif /* _INCLUDE__BASE__IPC_CALL_QUEUE_H_ */
