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
	/* perform IPC send operation */
	/*Not working implementation of Ipc_destination as class
	if(_dst.dst().snd_phone() < 1 ||
		Spartan::ipc_data_write_start_synch(_dst.dst().snd_phone(), _snd_msg->buf,
			_snd_msg->size()) != 0 ) {
	*/
	/*
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
/*	Not working implementation of Ipc_destination as class
	int s_phone = Spartan::ipc_connect_me_to(PHONE_CORE, dst.dst().rcv_task_id(),
		dst.dst().rcv_thread_id(), 0);
	Genode::printf("sendwait:\t[ostream] got phone nr %i\n", s_phone);

	_dst = Native_capability( Ipc_destination(dst.dst().rcv_task_id(),
				dst.dst().rcv_thread_id(), Spartan::task_get_id(),
				Spartan::thread_get_id(), s_phone),
			dst.local_name());
*/
	/*
	 * Establish connection to the other task before communicating
	 */
	int snd_phone = Spartan::ipc_connect_me_to(PHONE_CORE, dst.dst().rcv_task_id,
		dst.dst().rcv_thread_id, 0);

	/*
	 * Overwrite current Destination with new one where the phone_id
	 * for the connection is added (there is no possibility to simply add
	 * the phone_id to the existing _dst)
	 */
	Ipc_destination dest = {dst.dst().rcv_task_id, dst.dst().rcv_thread_id, snd_phone};
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
	Native_ipc_callid in_callid;

	/* 
	 * Wait for an incomming connection request if strea, is not connected
	 */
	if(!connected) {
		addr_t retval;

		in_callid = Spartan::ipc_wait_for_call_timeout(&call, 0);

		switch(IPC_GET_IMETHOD(call)) {
			// TODO replace IPC_M_CONNECT_ME_TO call with cloning of connection
			case IPC_M_CONNECT_ME_TO:
				/* Accept the connection
				 * TODO: make shure the sender has the right Capability ?
				 */
				retval = Spartan::ipc_answer_0(in_callid, 0);
				if(retval == 0) {
					/* Not working implementation of Ipc_destination as class
					Ipc_destination dest(dst().rcv_task_id(),
						dst().rcv_thread_id(),
						call.in_task_id, in_thread_id, 0,
						call.in_phone_hash);
					* TODO
					 * the new Ipc_destination object has to be injected into
					 * the Native_capability.
					 * Currently cant, because of missing interface
					 */

					_rcv_msg->set_task_id(call.in_task_id);
					_rcv_msg->set_thread_id(in_thread_id);
					_rcv_msg->set_phone_hash(call.in_phone_hash);
					_rcv_msg->set_callid(in_callid);

					connected = true;
				}
				break;
			default:
				/* Reject connection */
				 Spartan::ipc_answer_0(in_callid, -1);
		}
	}

	/*
	 * Wait for IPC message
	 */
	if(!Spartan::ipc_data_write_receive_timeout(&in_callid, &call, 		//
				&in_thread_id, &size, 0)			//
		/* Not working implementation of Ipc_destination as class
		|| (call.in_task_id != dst().snd_task_id())
		|| (in_thread_id != dst().snd_thread_id())
		*/
			|| (call.in_task_id != _rcv_msg->snd_task_id())		//
// TODO			|| (in_thread_id != _rcv_msg->snd_thread_id())		//
			|| (in_callid != _rcv_msg->callid())) {			//
		Spartan::ipc_answer_0(in_callid, -1);				//
		return;								// TODO
	}									// ->
										// has to be replaced with
	/* Retrieve the message */						// worker thread specific code
	Spartan::ipc_data_write_finalize(in_callid, _rcv_msg->buf, size);	//

	/* reset unmarshaller */
	_read_offset = sizeof(umword_t);
}


Ipc_istream::Ipc_istream(Msgbuf_base *rcv_msg)
:
	Ipc_unmarshaller(&rcv_msg->buf[0], rcv_msg->size()),
	Native_capability( Ipc_destination{Spartan::task_get_id(), 
			Spartan::thread_get_id(), -1}, 0),
	_rcv_msg(rcv_msg)
{
	_rcv_cs = 0;
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
