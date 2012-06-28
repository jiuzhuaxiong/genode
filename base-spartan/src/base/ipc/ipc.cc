/*
 * \brief  IPC implementation for OKL4
 * \author Norman Feske
 * \date   2009-03-25
 */

/*
 * Copyright (C) 2009-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <base/printf.h>
#include <base/ipc.h>
#include <base/native_types.h>
#include <base/blocking.h>

#include <spartan/syscalls.h>

using namespace Genode;

enum {
	PHONE_CORE = 0,
};

/*****************
 ** Ipc_ostream **
 *****************/

void Ipc_ostream::_send()
{
	/* perform IPC send operation
	 *
	 * Check whether the phone_id is valid and send the message
	 */
	if(_dst.dst().snd_phone < 1 ||
		Spartan::ipc_data_write_start_synch(_dst.dst().snd_phone, _snd_msg->buf,
			_snd_msg->size()) != 0 ) {
		PERR("ipc error in _send.");
		throw Genode::Ipc_error();
	}

	_write_offset = sizeof(umword_t);
}


Ipc_ostream::Ipc_ostream(Native_capability dst, Msgbuf_base *snd_msg)
:
	Ipc_marshaller(&snd_msg->buf[0], snd_msg->size()),
	_snd_msg(snd_msg), _dst(dst)
{
	/*
	 * Establish connection to the other task before communicating
	 */
	int snd_phone = Spartan::ipc_connect_me_to(PHONE_CORE, dst.dst().rcv_task_id,
		dst.dst().rcv_thread_id, Spartan::thread_get_id());

	/*
	 * Overwrite current Destination with new one where the phone_id
	 * for the connection is added (there is no possibility to simply add
	 * the phone_id to the existing _dst)
	 */
	Ipc_destination dest = {dst.dst().rcv_task_id, dst.dst().rcv_thread_id,
		Spartan::task_get_id(), Spartan::thread_get_id(), snd_phone};
	_dst = Native_capability( dest, dst.local_name() );

	_write_offset = sizeof(umword_t);
}


/*****************
 ** Ipc_istream **
 *****************/

void Ipc_istream::_wait()
{
	static bool		connected = false;
	Native_ipc_call		call;
	Native_thread_id	in_thread_id;
	addr_t			size;

	/* 
	 * Wait for an incomming connection request if strea, is not connected
	 */
	if(!connected) {
		addr_t retval;
		Native_ipc_call		rec_call;
		Native_thread_id	rec_thread_id;

		_rcv_msg->callid = Spartan::ipc_wait_for_call_timeout(&rec_call, 0);

		switch(IPC_GET_IMETHOD(rec_call)) {
			// TODO replace IPC_M_CONNECT_ME_TO call with cloning of connection
			case IPC_M_CONNECT_ME_TO:
				/* Accept the connection
				 * TODO: make shure the sender has the right Capability ?
				 */
				retval = Spartan::ipc_answer_0(_rcv_msg->callid, 0);
				if(retval == 0) {
					_dst.snd_task_id = rec_call.in_task_id;
					_dst.snd_thread_id = rec_thread_id;
					_dst.snd_phonehash = 0;

					connected = true;
				}
				break;
			default:
				/* no connection call -> reject connection */
				 Spartan::ipc_answer_0(_rcv_msg->callid, -1);
		}
	}

	/*
	 * Wait for IPC message
	 */
	if(!Spartan::ipc_data_write_receive_timeout(&_rcv_msg->callid, &call, 	//
				&in_thread_id, &size, 0)			//
		|| (call.in_task_id != _dst.snd_task_id)		//
		|| (in_thread_id != _dst.snd_thread_id)			//
		|| ((_dst.snd_phonehash != 0) &&
			(call.in_phone_hash != _dst.snd_phonehash))) {
		/* unknown sender */
		Spartan::ipc_answer_0(_rcv_msg->callid, -1);			//
		return;								// TODO
	}									// ->
	if(_dst.snd_phonehash == 0)
		_dst.snd_phonehash = call.in_phone_hash;
										// has to be replaced with
	/* Retrieve the message */						// worker thread specific code
	Spartan::ipc_data_write_finalize(_rcv_msg->callid, _rcv_msg->buf, size);	//

	/* reset unmarshaller */
	_read_offset = sizeof(umword_t);
}


Ipc_istream::Ipc_istream(Msgbuf_base *rcv_msg)
:
	Ipc_unmarshaller(&rcv_msg->buf[0], rcv_msg->size()),
	Native_capability(),
	_rcv_msg(rcv_msg)
{
	_rcv_cs = 0;
	_dst.rcv_task_id = Spartan::task_get_id();
	_dst.rcv_thread_id = Spartan::thread_get_id();
	_dst.snd_phonehash = 0;
	_read_offset = sizeof(umword_t);
}

Ipc_istream::~Ipc_istream() { }

Ipc_server::Ipc_server(Genode::Msgbuf_base *snd_msg, Genode::Msgbuf_base *rcv_msg)
:
	Ipc_istream(rcv_msg),
	Ipc_ostream(Native_capability(), snd_msg)
{}

void Ipc_server::_wait()
{}
