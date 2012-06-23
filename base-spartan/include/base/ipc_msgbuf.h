/*
 * \brief  OKL4-specific layout of IPC message buffer
 * \author Norman Feske
 * \date   2009-03-25
 *
 */

/*
 * Copyright (C) 2009-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__BASE__IPC_MSGBUF_H_
#define _INCLUDE__BASE__IPC_MSGBUF_H_

namespace Genode {

	/**
	 * IPC message buffer layout
	 */
	class Msgbuf_base
	{
		protected:

			size_t _size;
			char   _msg_start[];  /* symbol marks start of message */

			Native_task		_task_id;
			Native_thread		_thread_id;
			addr_t			_phone_hash;
			Native_ipc_callid 	_callid;

		public:

			/*
			 * Begin of actual message buffer
			 */
			char buf[];

			/**
			 * Return size of message buffer
			 */
			inline size_t size() const { return _size; };

			/**
			 * Return address of message buffer
			 */
			inline void *addr() { return &_msg_start[0]; };

			void set_task_id(Native_task id) { _task_id = id; }
			void set_thread_id(Native_thread id) { _thread_id = id; }
			void set_phone_hash(addr_t ph) { _phone_hash = ph; }
			void set_callid(Native_ipc_callid id) { _callid = id; }

			Native_task snd_task_id() { return _task_id; }
			Native_thread snd_thread_id() { return _thread_id; }
			addr_t snd_phone_hash() { return _phone_hash; }
			Native_ipc_callid callid() { return _callid; }
	};


	/**
	 * Instance of IPC message buffer with specified buffer size
	 */
	template <unsigned BUF_SIZE>
	class Msgbuf : public Msgbuf_base
	{
		public:

			char buf[BUF_SIZE];

			Msgbuf() { _size = BUF_SIZE; }
	};
}

#endif /* _INCLUDE__BASE__IPC_MSGBUF_H_ */
