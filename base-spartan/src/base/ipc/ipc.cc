/*
 * \brief  IPC implementation for Spartan
 * \author Norman Feske
 * \author tobias Börtitz
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
	PHONE_TO_MYSELF = 1,
};

/**********************
 ** helper functions **
 **********************/

extern "C++" Thread_utcb* _obtain_utcb();

addr_t _send_capability(Native_capability dest_cap, Native_capability snd_cap)
{
	return Spartan::ipc_send_phone(dest_cap.dst().snd_phone,
	                               snd_cap.dst().snd_phone,
	                               snd_cap.local_name(),
	                               snd_cap.dst().rcv_thread_id,
	                               dest_cap.dst().rcv_thread_id,
	                               _obtain_utcb()->thread_id());
}


bool _receive_capability(Msgbuf_base *rcv_msg)
{
	Ipc_message msg = _obtain_utcb()->wait_for_call(
	                                                 IPC_M_CONNECTION_CLONE);

	Ipc_destination dest = {msg.target_thread_id(), msg.cloned_phone()};
	Native_capability  cap = Native_capability(dest, msg.capability_id());
	Spartan::ipc_answer_0(msg.callid(), msg.snd_thread_id(), 0);

	return rcv_msg->cap_append(cap);
}


Native_ipc_callid
__send(Native_capability dst_cap, Msgbuf_base *snd_msg,
                         Native_thread_id my_tid, addr_t rep_callid=0)
{
	Ipc_message       rpl_msg;
	Native_ipc_callid snd_callid;
	Native_ipc_callid cap_callid[Msgbuf_base::MAX_CAP_ARGS];

	/* insert number of capabilities to be send into msgbuf */
//	PDBG("_snd_msg->buf[0]=%i, _snd_msg->cap_count()=%lu", _snd_msg->buf[0], _snd_msg->cap_count());
	snd_msg->buf[0] = snd_msg->cap_count();
	/* perform IPC send operation
	 *
	 * Check whether the phone_id is valid and send the message
	 */
	snd_callid = Spartan::ipc_data_write_slow(dst_cap.dst().snd_phone,
	                                          snd_msg->buf, snd_msg->size(),
	                                          dst_cap.dst().rcv_thread_id,
	                                          my_tid, rep_callid);

	rpl_msg = _obtain_utcb()->wait_for_answer(snd_callid);
	if(rpl_msg.answer_code() != EOK) {
		PERR("ipc error in _send [ErrorCode: %lu]",
		     rpl_msg.answer_code());
		throw Genode::Ipc_error();
	}
	snd_msg->callid = snd_callid;

	/* After sending the message itself, send all pending 
	 * capabilities (clone phones) */
	for(addr_t i=0; i<snd_msg->cap_count(); i++) {
		Native_capability snd_cap;
		if(!snd_msg->cap_get_by_order(i, &snd_cap))
			continue;
		cap_callid[i] = _send_capability(dst_cap, snd_cap);
	}
	/* now wait for all answers to the sent capabilities */
	for(addr_t i=0; i<snd_msg->cap_count(); i++) {
		rpl_msg = _obtain_utcb()->wait_for_answer(
		           cap_callid[i]);
		if(rpl_msg.answer_code() != EOK) {
			PERR("ipc error while sending capabilities "
			     "[ErrorCode: %lu]", rpl_msg.answer_code());
			throw Genode::Ipc_error();
		}
	}

	return snd_callid;
}

Native_ipc_callid
__wait(Msgbuf_base* rcv_msg, addr_t rep_callid=0)
{
	Ipc_message      msg;
	addr_t           size;
	Native_thread_id rcv_thread_id;

	/*
	 * Wait for IPC message
	 */
	msg = _obtain_utcb()->wait_for_call(IPC_M_DATA_WRITE,
	                                                   rep_callid);
	if(msg.method() != IPC_M_DATA_WRITE) {
		/* unknown sender */
		PDBG("wrong call method (msg.call_method()!=IPC_M_DATA_WRITE)!\n");
		Spartan::ipc_answer_0(msg.callid(), msg.snd_thread_id(), -1);
		return 0;
	}
	rcv_msg->callid = msg.callid();
	size = msg.msg_size();

	/* Retrieve the message */
	/* TODO compare send message size with my own message size
	 * -> which policy msgld be implemented?
	 */
	Spartan::ipc_data_write_accept(msg.callid(), rcv_msg->buf, size,
	                               msg.snd_thread_id());
//	PDBG("Ipc_istream:\twrite finalize returned %i\n", ret);

	/* set dst so it can be used in Ipc_server */
	rcv_thread_id = msg.snd_thread_id();

	/* extract all retrieved capailities */
//	PDBG("_rcv_msg->buf[0] = %i\n", _rcv_msg->buf[0]);
	for(int i=0; i<rcv_msg->buf[0]; i++) {
		_receive_capability(rcv_msg);
	}

	return rcv_thread_id;
}



/*****************
 ** Ipc_ostream **
 *****************/

void Ipc_ostream::_send()
{
	/* IPC send operation */
	__send(_dst, _snd_msg,
	       _obtain_utcb()->thread_id());

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
	/**
	 * wait for an incomming message
	 *  and set dst so it can be used in Ipc_server
	 */
	_dst.rcv_thread_id = __wait(_rcv_msg);;

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
	Native_ipc_callid snd_callid;
	Ipc_message       rpl_msg;

	/* send the request */
	Ipc_ostream::_send();

	/**
	 * clone the phone to myself to the server
	 *  so it can send back the reply
	 */
	snd_callid = Spartan::ipc_send_phone(Ipc_ostream::_dst.dst().snd_phone,
	                               PHONE_TO_MYSELF, 0,
	                               _obtain_utcb()->thread_id(),
	                               Ipc_ostream::_dst.dst().rcv_thread_id,
	                               _obtain_utcb()->thread_id());
	rpl_msg = _obtain_utcb()->wait_for_answer(snd_callid);
	if(rpl_msg.answer_code() != EOK) {
		PERR("ipc error in _send [ErrorCode: %lu]",
		     rpl_msg.answer_code());
		throw Genode::Ipc_error();
	}

	/* wait for the corresponding reply */
	__wait(Ipc_istream::_rcv_msg, _snd_msg->callid);
	Ipc_istream::_read_offset = sizeof(addr_t);
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

	_write_offset = 2*sizeof(umword_t);
	_read_offset = sizeof(umword_t);
}

void Ipc_server::_wait()
{
	Ipc_message msg;

	/* wait for new server request */
	Ipc_istream::_wait();

	/**
	 * wait for a cloned phone to arrive where the
	 *  reply is send over
	 */
	msg = _obtain_utcb()->wait_for_call(IPC_M_CONNECTION_CLONE,
	                                                   0);
	if(msg.method() != IPC_M_CONNECTION_CLONE) {
		/* unknown sender */
		PDBG("wrong call method (msg.call_method()!=IPC_M_DATA_WRITE)!\n");
		Spartan::ipc_answer_0(msg.callid(), msg.snd_thread_id(), -1);
		return;
	}
	Spartan::ipc_answer_0(msg.callid(), msg.snd_thread_id(), 0);

	/* define destination of next reply */
	Ipc_destination dest = { msg.snd_thread_id(),
	                         msg.cloned_phone() };
	Ipc_ostream::_dst = Native_capability( dest, Ipc_ostream::_dst.local_name() );

	_prepare_next_reply_wait();
}

void Ipc_server::_reply()
{
	/* send the reply */
	__send(Ipc_ostream::_dst, Ipc_ostream::_snd_msg, 
	       _obtain_utcb()->thread_id(),
	       Ipc_istream::_rcv_msg->callid);
	Ipc_ostream::_write_offset = sizeof(addr_t);

	/* hangup the phone the reply is sent over */
	Spartan::ipc_hangup(Ipc_ostream::_dst.dst().snd_phone,
	                    Ipc_ostream::_dst.dst().rcv_thread_id,
	                    _obtain_utcb()->thread_id());

	_prepare_next_reply_wait();
}

void Ipc_server::_reply_wait()
{
	if (_reply_needed)
		_reply();

	_wait();

	_prepare_next_reply_wait();
}

Ipc_server::Ipc_server(Msgbuf_base *snd_msg, Msgbuf_base *rcv_msg)
:
	Ipc_istream(rcv_msg),
	Ipc_ostream(Native_capability(), snd_msg)
{ }
