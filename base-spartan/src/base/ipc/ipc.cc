/*
 * \brief  IPC implementation for Spartan
 * \author Norman Feske
 * \author Tobias Börtitz
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

#include <util/string.h>

#include <spartan/ipc.h>
//#include <spartan/methods.h>

using namespace Genode;

enum {
	PHONE_CORE = 0,
};

/**********************
 ** helper functions **
 **********************/

char* _as_area=0;
void* _shared_area=0;

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
	Thread_utcb* my_utcb = _obtain_utcb();
	Ipc_message msg = my_utcb->msg_queue()->wait_for_call(
	                  my_utcb->thread_id(), IPC_M_CONNECTION_CLONE);

	Ipc_destination dest = {msg.target_thread_id(), msg.cloned_phone()};
	Native_capability  cap = Native_capability(dest, msg.capability_id());
	Spartan::ipc_answer_0(msg.callid(), msg.snd_thread_id(), 0);

	return rcv_msg->cap_append(cap);
}


void
__send_caps(Msgbuf_base *snd_msg, Native_capability dst_cap)
{
	Native_ipc_callid cap_callid[Msgbuf_base::MAX_CAP_ARGS];
	Ipc_message       rpl_msg;

	Thread_utcb* my_utcb = _obtain_utcb();

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
		rpl_msg = my_utcb->msg_queue()->wait_for_answer(
		           my_utcb->thread_id(), cap_callid[i]);
		if(rpl_msg.answer_code() != EOK) {
			PERR("ipc error while sending capabilities "
			     "[ErrorCode: %lu]", rpl_msg.answer_code());
			throw Genode::Ipc_error();
		}
	}
}


/* wait for incomming capabilities */
void
__rec_caps(Msgbuf_base* rcv_msg)
{
	/* extract all retrieved capailities */
	for(int i=0; i<rcv_msg->buf[0]; i++) {
		_receive_capability(rcv_msg);
	}
}



/*****************
 ** Ipc_ostream **
 *****************/

void Ipc_ostream::_send()
{
	Ipc_message       rpl_msg;
	Native_ipc_callid snd_callid;

	Thread_utcb* my_utcb = _obtain_utcb();
//	if(!_snd_msg->as_area) {
//		PERR("No address space area to send!");
//		return;
//	}

	/* insert number of capabilities to be send into msgbuf */
	_sndbuf[0] = _snd_msg->cap_count();
//	for(int i=0; i<50; i++)
//		printf("%lu-", _sndbuf[i]);
//	printf("\n");
//	memcpy(_snd_msg->as_area, _snd_msg->buf, _snd_msg->size());
	/* IPC send operations */

	/* TODO:
	 * evil hack, that only works when sending messages inside the same task
	 * save the buffer to be shared in a global available value
	 */
	_as_area = _sndbuf;

	/* share the buffer to the receipient */
	snd_callid = Spartan::ipc_share_segment(_dst.dst().snd_phone,
	                                        _sndbuf, _snd_msg->size(),
	                                        AS_AREA_READ | AS_AREA_WRITE,
	                                        _dst.dst().rcv_thread_id,
	                                        my_utcb->thread_id());

	rpl_msg = my_utcb->msg_queue()->wait_for_answer(my_utcb->thread_id(),
	                                                snd_callid);
	if(rpl_msg.answer_code() != EOK) {
		PERR("ipc error in _send [ErrorCode: %i]",
		     rpl_msg.answer_code());
		throw Genode::Ipc_error();
	}
	_snd_msg->callid = snd_callid;

	__send_caps(_snd_msg, _dst);

	_write_offset = sizeof(addr_t);
}


Ipc_ostream::Ipc_ostream(Native_capability dst, Msgbuf_base *snd_msg)
:
	Ipc_marshaller(&snd_msg->buf[0], snd_msg->size()),
	_snd_msg(snd_msg), _dst(dst)
{
//	snd_msg->buf = (char*)Spartan::as_area_create((void*) -1,
//	                               snd_msg->size(),
//	                               AS_AREA_READ | AS_AREA_WRITE);
//	PDBG("created buf is at %lu with a size of %lu", snd_msg->buf, snd_msg->size());
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
	/**
	 * wait for an incomming message
	 *  and set dst so it can be used in Ipc_server
	 */
	Ipc_message      msg;

	Thread_utcb* my_utcb = _obtain_utcb();

	/*
	 * Wait for IPC message to share a buffer
	 */
	msg = my_utcb->msg_queue()->wait_for_call(my_utcb->thread_id(), IPC_M_SHARE_OUT);

	if(msg.method() != IPC_M_SHARE_OUT
	   || msg.arg3() != (AS_AREA_READ | AS_AREA_WRITE)) {
		/* unknown sender */
		PERR("wrong call method (msg.call_method()!=IPC_M_SHARE_OUT)!\n");
		Spartan::ipc_answer_0(msg.callid(), msg.snd_thread_id(), -1);
		return;
	}
	_rcv_msg->callid = msg.callid();

//	PDBG("PEWPEW puff=%lu, &puff=%lu", puff, &puff);
	/* receive the shared address space area to the _rcv_msg */
	addr_t ret = Spartan::ipc_share_accept(msg.callid(), &_rcv_msg->as_area, msg.arg5());
	/* TODO:
	 * part of the evil hack
	 * save the shared area, since it can't be used
	 */
	_shared_area = _rcv_msg->as_area;
//	PDBG("PEWPEW puff=%lu, &puff=%lu", puff, &puff);

	addr_t cp_size = _rcv_msg->size()<msg.arg2() ? _rcv_msg->size():msg.arg2();
	/* TODO:
	 * part of the evil hack
	 * copy the content of the saved client's sendbuffer to the
	 *  receiving buffer
	 *  originally should be copied from the shared adress space area
	 * afterwards save the fake shared address spacae area to the _rcv_msg
	 */
	memcpy(_rcvbuf, _as_area, cp_size);
	_rcv_msg->as_area = _as_area;

	__rec_caps(_rcv_msg);

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
	Native_ipc_callid cap_callid[Msgbuf_base::MAX_CAP_ARGS];
	Ipc_message       rpl_msg;

	Thread_utcb* my_utcb = _obtain_utcb();

	/* send the request */
	Ipc_ostream::_send();

	/* ask for the reply to be copied into the address space area */
	cap_callid[0] = Spartan::ipc_call_async_fast(Ipc_ostream::_dst.dst().snd_phone,
	                                             IPC_M_REPLY_READY, 0, 0,
	                                             my_utcb->thread_id(),
	                                             Ipc_ostream::_dst.dst().rcv_thread_id);
	rpl_msg = my_utcb->msg_queue()->wait_for_answer(my_utcb->thread_id(),
	                                                cap_callid[0]);
	if(rpl_msg.answer_code() != EOK) {
		PERR("ipc error in _send [ErrorCode: %i]",
		     rpl_msg.answer_code());
		throw Genode::Ipc_error();
	}
	/* copy the reply from the shared address space area */
	memcpy(_rcvbuf, _snd_msg->as_area, _rcv_msg->size());

	/* ask for the phones to the capabilities */
	for(int i=0; i<Ipc_istream::_rcv_msg->buf[0]; i++)
		Spartan::ipc_receive_phone(Ipc_ostream::_dst.dst().snd_phone, i,
		                           Ipc_ostream::_dst.dst().rcv_thread_id,
		                           _obtain_utcb()->thread_id());
	for(int i=0; i<Ipc_istream::_rcv_msg->buf[0]; i++) {
		Thread_utcb* my_utcb = _obtain_utcb();
		rpl_msg = my_utcb->msg_queue()->wait_for_answer(
		           my_utcb->thread_id(), cap_callid[i]);
		if(rpl_msg.answer_code() != EOK) {
			PERR("ipc error while sending capabilities "
			     "[ErrorCode: %lu]", rpl_msg.answer_code());
			throw Genode::Ipc_error();
		}
	}

	Ipc_istream::_read_offset = sizeof(addr_t);
}

Ipc_client::Ipc_client(Native_capability const &srv, Msgbuf_base *snd_msg,
                       Msgbuf_base *rcv_msg)
:
	Ipc_istream(rcv_msg),
	Ipc_ostream(srv, snd_msg), _result(0)
{
//	snd_msg->buf = (char*)Spartan::as_area_create((void*) -1, snd_msg->size(), AS_AREA_READ | AS_AREA_WRITE);
	_snd_msg->as_area = Spartan::as_area_create((void*) -1, snd_msg->size(), AS_AREA_READ | AS_AREA_WRITE);
	_sndbuf = (char*)_snd_msg->as_area;
//	PDBG("snd_msg->as_area = %lu", snd_msg->as_area);
}


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
	/* transfer the shared address space area to the sending message */
	_snd_msg->as_area = _rcv_msg->as_area;
	/* assign the shared address space area to the pointer the server writes to */
	_sndbuf = (char*)_snd_msg->as_area;

	/* define destination of next reply */
	Ipc_destination dest = { Ipc_istream::_dst.rcv_thread_id,
	                         Ipc_ostream::_dst.dst().snd_phone };
	Ipc_ostream::_dst = Native_capability( dest, Ipc_ostream::_dst.local_name() );

	_prepare_next_reply_wait();
}

void Ipc_server::_reply()
{
	Ipc_message msg;

	/* insert number of capabilities to be send into msgbuf */
	_sndbuf[0] = _snd_msg->cap_count();
//	for(int i=0; i<50; i++)
//		printf("%lu-", _sndbuf[i]);
//	printf("\n");
//	memcpy(_snd_msg->as_area, _snd_msg->buf, _snd_msg->size());
	/* wait for the question for the reply */
	msg = _obtain_utcb()->msg_queue()->wait_for_call(_obtain_utcb()->thread_id(),
	                                          IPC_M_REPLY_READY);
	Spartan::ipc_answer_0(msg.callid(), msg.snd_thread_id(), EOK);
	/* destroy the shared address space acrea */
	Spartan::as_area_destroy(_shared_area);

	/* wait for the question to clone capabilities */
	for(addr_t i=0; i<Ipc_ostream::_snd_msg->cap_count(); i++) {
		Thread_utcb* my_utcb = _obtain_utcb();
		msg = my_utcb->msg_queue()->wait_for_call(my_utcb->thread_id(),
		                                         IPC_M_CLONE_ESTABLISH);

		Native_capability snd_cap;
		if(Ipc_ostream::_snd_msg->cap_get_by_order(msg.arg2(), &snd_cap))
			Spartan::ipc_answer_3(msg.callid(), msg.snd_thread_id(),
			                      EOK, snd_cap.dst().snd_phone,
			                      snd_cap.local_name(),
			                      snd_cap.dst().rcv_thread_id);
		else
			Spartan::ipc_answer_0(msg.callid(), msg.snd_thread_id(),
			                      EREFUSED);
	}
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
