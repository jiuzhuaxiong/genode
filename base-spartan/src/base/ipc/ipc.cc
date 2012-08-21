/*
 * \brief  IPC implementation for Spartan
 * \author Tobias Börtitz
 * \date   20122-08-14
 */

/*
 * Copyright (C) 2009-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/printf.h>
#include <base/ipc.h>
#include <base/native_types.h>
#include <base/ipc_call.h>
#include <base/thread.h>

/* Spartan includes */
#include <spartan/syscalls.h>

using namespace Genode;


enum {
	PHONE_CORE = 0,
};


/**********************
 ** helper functions **
 **********************/

int _send_capability(Native_capability dest_cap, Native_capability snd_cap)
{
	return Spartan::ipc_clone_connection(dest_cap.dst().snd_phone,
	                                     dest_cap.dst().rcv_thread_id,
	                                     snd_cap.dst().rcv_thread_id,
	                                     snd_cap.local_name(),
	                                     snd_cap.dst().snd_phone);
}


bool _receive_capability(Msgbuf_base *rcv_msg)
{
	Ipc_call call = Thread_base::myself()->utcb()->wait_for_call(
	                 IPC_M_CONNECTION_CLONE);

	Ipc_destination  dest = {call.target_thread_id(), call.cloned_phone()};
	Native_capability cap = Native_capability(dest, call.capability_id());
	Spartan::ipc_answer_0(call.callid(), 0);

	PDBG("_receive_capability:\tincomming phone = %i", cap.dst().snd_phone);

	return rcv_msg->cap_append(cap);
}


/*****************
 ** Ipc_ostream **
 *****************/

void Ipc_ostream::_send()
{
	/* insert number of capabilities to be send into msgbuf */
	_snd_msg->buf[0] = _snd_msg->cap_count();
	PDBG("_snd_msg->buf[0]=%i, _snd_msg->cap_count()=%lu", _snd_msg->buf[0], _snd_msg->cap_count());
	/* perform IPC send operation
	 *
	 * Check whether the phone_id is valid and send the message
	 */
	PDBG("thread %lu: sending via phone %i to thread %lu", Spartan::thread_get_id(), _dst.dst().snd_phone, _dst.dst().rcv_thread_id);
	if(_dst.dst().snd_phone < 1 ||
		Spartan::ipc_data_write_start_synch(_dst.dst().snd_phone,
			_dst.dst().rcv_thread_id, _snd_msg->buf, _snd_msg->size()) != 0 ) {
		PERR("ipc error in _send.");
		throw Genode::Ipc_error();
	}
	/* After sending the message itself, send all pending 
	 * capabilities (clone phones) */
	for(addr_t i=0; i<_snd_msg->cap_count(); i++) {
		Native_capability snd_cap;
		if(!_snd_msg->cap_get_by_order(i, &snd_cap))
			continue;
		_send_capability(_dst, snd_cap);
	}

	_write_offset = sizeof(addr_t);
}


Ipc_ostream::Ipc_ostream(Native_capability dst, Msgbuf_base *snd_msg)
:
	Ipc_marshaller(&snd_msg->buf[0], snd_msg->size()),
	_snd_msg(snd_msg), _dst(dst)
{
	/* write dummy for later to be inserted
	 * number of capabilities to be send */
	_write_to_buf((addr_t)0);
	_write_offset = sizeof(addr_t);
}


/*****************
 ** Ipc_istream **
 *****************/

void Ipc_istream::_wait()
{
	Ipc_call call;
	addr_t   size;

	/* Wait for IPC message */
	call = Thread_base::myself()->utcb()->wait_for_call();
	printf("Ipc_istream:\tgot call with callid %lu\n", call.callid());
	if(call.call_method() != IPC_M_DATA_WRITE) {
		/* wrong method send */
		printf("Ipc_istream:\twrong call method (call.call_method()!=IPC_M_DATA_WRITE)!\n");
		Spartan::ipc_answer_0(call.callid(), -1);
		return;
	}
	_rcv_msg->callid = call.callid();
	size = call.msg_size();

	/* Retrieve the message */
	/* TODO compare send message size with my own message size
	 * -> which policy should be implemented?
	 */
	int ret = Spartan::ipc_data_write_finalize(_rcv_msg->callid, _rcv_msg->buf, size);
	PDBG("Ipc_istream:\twrite finalize returned %i\n", ret);

	/* set dst so it can be used in Ipc_server */
	_dst.rcv_thread_id = call.snd_thread_id();

	/* extract all retrieved capailities */
	PDBG("Ipc_istream:\t_rcv_msg->buf[0] = %i\n", _rcv_msg->buf[0]);
	for(int i=0; i<_rcv_msg->buf[0]; i++) {
		_receive_capability(_rcv_msg);
	}

	/* reset unmarshaller */
	_read_offset = sizeof(addr_t);
}


Ipc_istream::Ipc_istream(Msgbuf_base *rcv_msg)
:
	Ipc_unmarshaller(&rcv_msg->buf[0], rcv_msg->size()),
	Native_capability(),
	_rcv_msg(rcv_msg)
{
	_rcv_cs = 0;
	_read_offset = sizeof(addr_t);
}

Ipc_istream::~Ipc_istream() { }

/****************
 ** Ipc_client **
 ****************/

void Ipc_client::_call()
{
	Ipc_ostream::_send();
	/**
	 * request the answer from the server
	 * Check whether the phone_id is valid and request the message
	 */
	if(Ipc_ostream::_dst.dst().snd_phone < 1 ||
		Spartan::ipc_data_read_start_synch(Ipc_ostream::_dst.dst().snd_phone,
			Ipc_ostream::_dst.dst().rcv_thread_id,
			Ipc_istream::_rcv_msg->buf, _rcv_msg->size()) != 0 ) {
		PERR("ipc error in _call.");
		throw Genode::Ipc_error();
	}

	/* retrieve all send Capabilites */
	for(int i=0; i<_rcv_msg->buf[0]; i++) {
		_receive_capability(_rcv_msg);
	}
	_read_offset = sizeof(addr_t);
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
	_write_offset = 2*sizeof(addr_t);

	/* receive buffer offset */
	_read_offset = sizeof(addr_t);
}


void Ipc_server::_wait()
{
	/* wait for new server request */
	Ipc_istream::_wait();

	/* define destination of next reply */
	Ipc_destination dest = { Ipc_istream::_dst.rcv_thread_id,
	                         Ipc_ostream::_dst.dst().snd_phone };
	Ipc_ostream::_dst = Native_capability( dest, Ipc_ostream::_dst.local_name() );

	_prepare_next_reply_wait();
}


void Ipc_server::_reply()
{
	Ipc_call call = Thread_base::myself()->utcb()->get_reply();
	while(call.callid() == 0)
		call = Thread_base::myself()->utcb()->get_reply();

	if(call.call_method() != IPC_M_DATA_READ) {
		/* wrong method send */
		printf("Ipc_istream:\twrong call method (call.call_method()!=IPC_M_DATA_READ)!\n");
		Spartan::ipc_answer_0(call.callid(), -1);
		return;
	}
	addr_t size = call.msg_size();

	/* Send the message */
	/* TODO compare send message size with my own message size
	 * -> which policy should be implemented?
	 */
	int ret = Spartan::ipc_data_read_finalize(call.callid(), _snd_msg->buf, size);
	printf("Ipc_server:\tread finalize returned %i\n", ret);
	_prepare_next_reply_wait();
}


void Ipc_server::_reply_wait()
{
	if (_reply_needed)
		_reply();

	_wait();
}


Ipc_server::Ipc_server(Genode::Msgbuf_base *snd_msg, Genode::Msgbuf_base *rcv_msg)
:
	Ipc_istream(rcv_msg),
	Ipc_ostream(Native_capability(), snd_msg)
{}

