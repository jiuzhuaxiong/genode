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
#include <base/ipc_call.h>
#include <base/ipc_manager.h>

#include <spartan/syscalls.h>

using namespace Genode;

enum {
	PHONE_CORE = 0,

	IPC_CONNECTION_REQUEST = 30,
};

/**********************
 ** helper functions **
 **********************/

/* establish a connection to another task, therefore:
 * - ask a nameserver to be connected to a task
 * - nameserver answers whether or not it can connect to the other task/thread
 * - if answer was positive: wait for a cloned connection to come in
 */
int request_connection(int snd_phone, Native_task dst_task_id,
		Native_thread_id dst_thread_id)
{
	int		ret;
	Ipc_call	call;
	ret = Spartan::ipc_call_sync_3_0(snd_phone, IPC_CONNECTION_REQUEST,
			dst_task_id, dst_thread_id, Spartan::thread_get_id());

	/* connection endpoint is not known */
	if(ret<0)
		return ret;
	/* in order to send a cloned connection
	 * the sender needs a temporary phone to us */
	if(ret>0) {
		ret = Spartan::ipc_connect_to_me(snd_phone, Spartan::thread_get_id(),
				0, 0, 0, 0);
		/* exit if the neededconnection could not be established */
		if(ret != 0)
			return -1;
	}

	call = Ipc_manager::singleton()->wait_for_call(Spartan::task_get_id(),
			Spartan::thread_get_id(), IPC_M_CONNECTION_CLONE);
	ret = call.cloned_phone();
	Spartan::ipc_answer_0(call.callid(), 0);

	/* return the aquired phone */
	return ret;
}


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
		Spartan::ipc_data_write_start_synch(_dst.dst().snd_phone,
			_dst.dst().rcv_task_id, _dst.dst().rcv_thread_id,
			_snd_msg->buf, _snd_msg->size()) != 0 ) {
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
/* old implementation not using connection cloning - not longer used but will
 * remain here for a while for debugging purposes
 *
	int snd_phone = Spartan::ipc_connect_me_to(PHONE_CORE, dst.dst().rcv_task_id,
		dst.dst().rcv_thread_id, Spartan::thread_get_id());
*/
	int clone_phone = request_connection(PHONE_CORE, dst.dst().rcv_task_id,
			dst.dst().rcv_thread_id);
//	printf("Ipc_ostream:\t snd_phone=%i, clone_phone=%i\n", snd_phone, clone_phone);
	/*
	 * Overwrite current Destination with new one where the phone_id
	 * for the connection is added (there is no possibility to simply add
	 * the phone_id to the existing _dst)
	 */
	Ipc_destination dest = {dst.dst().rcv_task_id, dst.dst().rcv_thread_id,
		Spartan::task_get_id(), Spartan::thread_get_id(), clone_phone};
	_dst = Native_capability( dest, dst.local_name() );

	_write_offset = sizeof(umword_t);
}


/*****************
 ** Ipc_istream **
 *****************/

void Ipc_istream::_wait()
{
//	static bool		connected = false;
	Ipc_call		call;
	addr_t			size;

	/*
	 * Wait for an incomming connection request if strea, is not connected
	 *
	 * is not longer used but code will remain here for a while
	 * for debugging purspose
	while(!connected) {
		addr_t retval;
		call = Ipc_manager::singleton()->wait_for_call(Spartan::task_get_id(),
				Spartan::thread_get_id());
		printf("Ipc_istream:\tUNCONNECTED got call with callid %lu\n", call.callid());
		switch(call.call_method()) {
			case IPC_M_CONNECT_ME_TO:
				// Call is not for me -> reject request
				if(call.dest_task_id() != Spartan::task_get_id()
					|| call.dest_thread_id() != Spartan::thread_get_id()) {
						Spartan::ipc_answer_0(call.callid(), -1);
						break;
				}

				// Accept the connection
				retval = Spartan::ipc_answer_0(call.callid(), 0);
				printf("Ipc_istream:\t asnwering the connection request gave %lu\n", retval);
				if(retval == 0) {
					_rcv_msg->callid = call.callid();
					_dst.snd_task_id = call.snd_task_id();
					_dst.snd_thread_id = call.snd_thread_id();
					_dst.snd_phonehash = 0;

					connected = true;
				}
				break;
			default:
				// no connection call -> reject call
				 Spartan::ipc_answer_0(_rcv_msg->callid, -1);
		}
	}
*/
	/*
	 * Wait for IPC message
	 */
	call = Ipc_manager::singleton()->wait_for_call(Spartan::task_get_id(),
			Spartan::thread_get_id());
	printf("Ipc_istream:\tgot call with callid %lu\n", call.callid());
	if(call.call_method() != IPC_M_DATA_WRITE) {
		/* since connection cloning is used we do not know who is supposed/allowed
		 * to send data
		 * code will remain for debugging purpose
		|| (call.snd_task_id() != _dst.snd_task_id)
		|| (call.snd_thread_id() != _dst.snd_thread_id)
		|| ((_dst.snd_phonehash != 0) &&
			(call.snd_phonehash() != _dst.snd_phonehash))) {
		*/
		/* unknown sender */
		printf("Ipc_istream:\twrong call method (call.call_method()!=IPC_M_DATA_WRITE)!\n");
		Spartan::ipc_answer_0(_rcv_msg->callid, -1);
		return;
	}
	_rcv_msg->callid = call.callid();
	size = call.call_arg2();

	_dst.snd_task_id = call.snd_task_id();
	_dst.snd_thread_id = call.snd_thread_id();
	_dst.snd_phonehash = call.snd_phonehash();

	/* Retrieve the message */
	int ret = Spartan::ipc_data_write_finalize(_rcv_msg->callid, _rcv_msg->buf, size);
	printf("Ipc_istream:\twrite finalize returned %i\n", ret);

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

/****************
 ** Ipc_client **
 ****************/

void Ipc_client::_call()
{
	Ipc_ostream::_send();
	Ipc_istream::_wait();
}

Ipc_client::Ipc_client(Native_capability const &srv,
                       Msgbuf_base *snd_msg, Msgbuf_base *rcv_msg)
: Ipc_istream(rcv_msg), Ipc_ostream(srv, snd_msg), _result(0)
{ }


/****************
 ** Ipc_server **
 ****************/

void Ipc_server::_prepare_next_reply_wait()
{
	/* now we have a request to reply */
	_reply_needed = true;

	/* leave space for return value at the beginning of the msgbuf */
	_write_offset = 2*sizeof(umword_t);

	/* receive buffer offset */
	_read_offset = sizeof(umword_t);
}

void Ipc_server::_wait()
{
	/* wait for new server request */
	Ipc_istream::_wait();

	/* define destination of next reply */
	Ipc_destination dest = {Ipc_istream::_dst.snd_task_id,
			Ipc_istream::_dst.snd_thread_id,
			Spartan::task_get_id(),
			Spartan::thread_get_id(),
			Ipc_ostream::_dst.dst().snd_phone,
			Ipc_ostream::_dst.dst().snd_phonehash};
	Ipc_ostream::_dst = Native_capability( dest, Ipc_ostream::_dst.local_name() );

	_prepare_next_reply_wait();
}

void Ipc_server::_reply()
{
	Ipc_ostream::_send();

	_prepare_next_reply_wait();
}

void Ipc_server::_reply_wait()
{
	if (_reply_needed)
		_reply();
	else
		_wait();
}

Ipc_server::Ipc_server(Genode::Msgbuf_base *snd_msg, Genode::Msgbuf_base *rcv_msg)
:
	Ipc_istream(rcv_msg),
	Ipc_ostream(Native_capability(), snd_msg)
{}

