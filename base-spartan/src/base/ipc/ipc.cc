
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

#include <base/ipc.h>
#include <base/native_types.h>
#include <base/ipc_message.h>
#include <base/ipc_manager.h>
#include <base/thread.h>

#include <spartan/ipc.h>

using namespace Genode;

enum {
	PHONE_CORE = 0,
};

/**********************
 ** helper functions **
 **********************/

addr_t _send_capability(Native_capability dest_cap, Native_capability snd_cap)
{
	return Spartan::ipc_send_phone(dest_cap.dst().snd_phone,
	                               snd_cap.dst().snd_phone,
	                               snd_cap.local_name(),
	                               snd_cap.dst().rcv_thread_id,
	                               dest_cap.dst().rcv_thread_id,
	                               Thread_base::myself()->utcb()->thread_id());
}

bool _receive_capability(Msgbuf_base *rcv_msg)
{
	Ipc_message msg = Thread_base::myself()->utcb()->wait_for_call(
	                  IPC_M_CONNECTION_CLONE);

	Ipc_destination dest = {msg.target_thread_id(), msg.cloned_phone()};
	Native_capability  cap = Native_capability(dest, msg.capability_id());
	Spartan::ipc_answer_0(msg.callid(), msg.snd_thread_id(), 0);

	return rcv_msg->cap_append(cap);
}


/*****************
 ** Ipc_ostream **
 *****************/

void Ipc_ostream::_send()
{
	Ipc_message rpl_msg;
	Native_ipc_callid snd_callid[Msgbuf_base::MAX_CAP_ARGS];
	/* insert number of capabilities to be send into msgbuf */
	_snd_msg->buf[0] = _snd_msg->cap_count();
//	PDBG("_snd_msg->buf[0]=%i, _snd_msg->cap_count()=%lu", _snd_msg->buf[0], _snd_msg->cap_count());
	/* perform IPC send operation
	 *
	 * Check whether the phone_id is valid and send the message
	 */
	snd_callid[0] = Spartan::ipc_data_write(_dst.dst().snd_phone,
	                                     _snd_msg->buf, _snd_msg->size(),
	                                     _dst.dst().rcv_thread_id,
	                                     Thread_base::myself()->utcb()->thread_id());

	rpl_msg = Thread_base::myself()->utcb()->wait_for_answer(snd_callid[0]);
	if(rpl_msg.answer_code() != EOK) {
		PERR("ipc error in _send [ErrorCode: %lu]",
		     rpl_msg.answer_code());
		throw Genode::Ipc_error();
	}

	/* After sending the message itself, send all pending 
	 * capabilities (clone phones) */
	for(addr_t i=0; i<_snd_msg->cap_count(); i++) {
		Native_capability snd_cap;
		if(!_snd_msg->cap_get_by_order(i, &snd_cap))
			continue;
		snd_callid[i] = _send_capability(_dst, snd_cap);
	}
	/* now wait for all answers to the sent capabilities */
	for(addr_t i=0; i<_snd_msg->cap_count(); i++) {
		rpl_msg = Ipc_manager::singleton()->my_utcb()->wait_for_answer(
		           snd_callid[i]);
		if(rpl_msg.answer_code() != EOK) {
			PERR("ipc error while sending capabilities "
			     "[ErrorCode: %lu]", rpl_msg.answer_code());
			throw Genode::Ipc_error();
		}
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

//void Ipc_istream::_wait() { }
void Ipc_istream::_wait()
{
	Ipc_message		msg;
	addr_t			size;

	/*
	 * Wait for IPC message
	 */
	msg = Thread_base::myself()->utcb()->wait_for_call(IPC_M_DATA_WRITE);
	if(msg.method() != IPC_M_DATA_WRITE) {
		/* unknown sender */
		PDBG("wrong call method (msg.call_method()!=IPC_M_DATA_WRITE)!\n");
		Spartan::ipc_answer_0(msg.callid(), msg.snd_thread_id(), -1);
		return;
	}
	_rcv_msg->callid = msg.callid();
	size = msg.msg_size();

	/* Retrieve the message */
	/* TODO compare send message size with my own message size
	 * -> which policy msgld be implemented?
	 */
	Spartan::ipc_data_write_accept(msg.callid(), _rcv_msg->buf, size,
	                         msg.snd_thread_id());
//	PDBG("Ipc_istream:\twrite finalize returned %i\n", ret);

	/* set dst so it can be used in Ipc_server */
	_dst.rcv_thread_id = msg.snd_thread_id();

	/* extract all retrieved capailities */
//	PDBG("_rcv_msg->buf[0] = %i\n", _rcv_msg->buf[0]);
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
	Ipc_message       rpl_msg;
	Native_ipc_callid snd_callid;

	Ipc_ostream::_send();

	snd_callid = Spartan::ipc_data_read(Ipc_ostream::_dst.dst().snd_phone,
	                                    Ipc_istream::_rcv_msg->buf,
	                                    Ipc_istream::_rcv_msg->size(),
	                                    Ipc_ostream::_dst.dst().rcv_thread_id,
	                                    Spartan::thread_get_id());

	rpl_msg = Thread_base::myself()->utcb()->wait_for_answer(snd_callid);
	if(rpl_msg.answer_code() != EOK) {
		PERR("ipc error in _call [ErrorCode: %lu]",
		     rpl_msg.answer_code());
		throw Genode::Ipc_error();
	}

//	PDBG("_rcv_msg->buf[0] = %i\n", Ipc_istream::_rcv_msg->buf[0]);
	for(int i=0; i<Ipc_istream::_rcv_msg->buf[0]; i++) {
		_receive_capability(Ipc_istream::_rcv_msg);
	}
	_read_offset = sizeof(addr_t);
}

Ipc_client::Ipc_client(Native_capability const &srv, Msgbuf_base *snd_msg,
                       Msgbuf_base *rcv_msg)
:
	Ipc_istream(rcv_msg),
	Ipc_ostream(srv, snd_msg), _result(0)
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
	Ipc_message       msg;
	addr_t            size;
	Native_ipc_callid snd_callid[Msgbuf_base::MAX_CAP_ARGS];

	/*
	 * Wait for IPC message
	 */
	msg = Thread_base::myself()->utcb()->wait_for_call(IPC_M_DATA_READ);
	if(msg.method() != IPC_M_DATA_READ) {
		/* unknown sender */
		PDBG("wrong call method (msg.call_method()!=IPC_M_DATA_READ)!\n");
		Spartan::ipc_answer_0(msg.callid(), msg.snd_thread_id(), -1);
		return;
	}
	size = msg.msg_size();

	/* Send the message */
	Spartan::ipc_data_read_accept(msg.callid(), Ipc_ostream::_snd_msg->buf,
	                              size,
	                              msg.snd_thread_id());
	/* TODO
	 * did it send all content (size == Ipc_ostream::_snd_msg->size()?) */

	/* After sending the message itself, send all pending 
	 * capabilities (clone phones) */
	for(addr_t i=0; i<Ipc_ostream::_snd_msg->cap_count(); i++) {
		Native_capability snd_cap;
		if(!_snd_msg->cap_get_by_order(i, &snd_cap))
			continue;
		snd_callid[i] = _send_capability(Ipc_ostream::_dst, snd_cap);
	}
	/* now wait for all answers to the sent capabilities */
	for(addr_t i=0; i<_snd_msg->cap_count(); i++) {
		msg = Ipc_manager::singleton()->my_utcb()->wait_for_answer(
		           snd_callid[i]);
		if(msg.answer_code() != EOK) {
			PERR("ipc error while sending capabilities "
			     "[ErrorCode: %lu]", msg.answer_code());
			throw Genode::Ipc_error();
		}
	}
}

void Ipc_server::_reply_wait() { }

Ipc_server::Ipc_server(Msgbuf_base *snd_msg, Msgbuf_base *rcv_msg)
:
	Ipc_istream(rcv_msg),
	Ipc_ostream(Native_capability(), snd_msg)
{ }
