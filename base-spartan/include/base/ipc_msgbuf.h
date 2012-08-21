/*
 * \brief  Spartan-specific layout of IPC message buffer
 * \author Norman Feske
 * \author Tobias Brtitz
 * \date   2012-08-14
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
		public:
			enum {
				MAX_CAP_ARGS = 4,
			};

		protected:

			size_t _size;
			char   _msg_start[];  /* symbol marks start of message */

			/* Capabilities to be send / received */
			Native_capability _caps[MAX_CAP_ARGS];

			/* Number of capabilities to be send / received */
			addr_t _cap_count;

		public:

			Native_ipc_callid callid;
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

			/**
			 * Reset all capabilities in buffer
			 */
			inline void cap_reset() { _cap_count = 0; };
			inline addr_t cap_count() { return _cap_count; };
			inline addr_t max_cap_count() { return MAX_CAP_ARGS; };
			bool cap_append(Native_capability cap)
			{
				if(_cap_count >= MAX_CAP_ARGS)
					return false;

				_caps[_cap_count++] = cap;
				return true;
			}
			bool cap_get_by_order(unsigned i, Native_capability *cap)
			{
				if(i < _cap_count) {
					*cap = _caps[i];
					return true;
				}

				return false;
			}
			bool cap_get_by_id(Native_capability *cap, long id)
			{
				for(addr_t i=0; i<_cap_count; i++)
					if(id == _caps[i].local_name()) {
						*cap = _caps[i];
						return true;
					}

				*cap = Native_capability();
				return false;
			}
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
