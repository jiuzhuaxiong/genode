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

#include <spartan/ipc.h>
//#include <spartan/methods.h>

using namespace Genode;

enum {
	PHONE_CORE = 0,
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
	Thread_utcb* my_utcb = _obtain_utcb();
	Ipc_message msg = my_utcb->msg_queue()->wait_for_call(
	                  my_utcb->thread_id(), IPC_M_CONNECTION_CLONE);

	Ipc_destination dest = {msg.target_thread_id(), msg.cloned_phone()};
	Native_capability  cap = Native_capability(dest, msg.capability_id());
	Spartan::ipc_answer_0(msg.callid(), msg.snd_thread_id(), 0);

	return rcv_msg->cap_append(cap);
}


Native_ipc_callid
__share_buf(Native_capability dst_cap, Msgbuf_base *snd_msg)
{
	Ipc_message       rpl_msg;
	Native_ipc_callid snd_callid;

	Thread_utcb* my_utcb = _obtain_utcb();

	snd_callid = Spartan::ipc_share_segment(dst_cap.dst().snd_phone,
	                                        snd_msg->buf, snd_msg->size(),
	                                        AS_AREA_READ | AS_AREA_WRITE,
	                                        dst_cap.dst().rcv_thread_id,
	                                        my_utcb->thread_id());

	rpl_msg = my_utcb->msg_queue()->wait_for_answer(my_utcb->thread_id(),
	                                                snd_callid);
	if(rpl_msg.answer_code() != EOK) {
		PERR("ipc error in _send [ErrorCode: %i]",
		     rpl_msg.answer_code());
		throw Genode::Ipc_error();
	}
	snd_msg->callid = snd_callid;

	for(int i=0; i<50; i++)
		printf("%lu;", snd_msg->buf[i]);
	printf("\n");

	return snd_callid;
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


/* wait for a message to read/write data from/to the sender of the call */
Native_thread_id
__rec_buf(Msgbuf_base* rcv_msg)
{
	Ipc_message      msg;
	Native_thread_id rcv_thread_id;
//	void* puff;

	Thread_utcb* my_utcb = _obtain_utcb();

	/*
	 * Wait for IPC message
	 */
	msg = my_utcb->msg_queue()->wait_for_call(my_utcb->thread_id(), IPC_M_SHARE_OUT);

	if(msg.method() != IPC_M_SHARE_OUT
	   || msg.arg3() != (AS_AREA_READ | AS_AREA_WRITE)) {
		/* unknown sender */
		PDBG("wrong call method (msg.call_method()!=IPC_M_SHARE_OUT)!\n");
		Spartan::ipc_answer_0(msg.callid(), msg.snd_thread_id(), -1);
		return 0;
	}
	rcv_msg->callid = msg.callid();
//	rcv_msg->size = msg.arg2();

	/* Retrieve the message */
	/* TODO compare send message size with my own message size
	 * -> which policy msgld be implemented?
	 */
	PDBG("OMNOMNOMNOMNOM size=%lu", msg.arg2());
	void* puff = rcv_msg->buf;
	PDBG("PEWPEW puff=%lu, &puff=%lu", puff, &puff);
	addr_t ret = Spartan::ipc_share_accept(msg.callid(), &puff, msg.arg5());
	for(int i=0; i<50; i++)
		printf("%lu:", rcv_msg->buf[i]);
	printf("| ret = %i\n", ret);
//	PDBG("Ipc_istream:\twrite finalize returned %i\n", ret);

	/* set dst so it can be used in Ipc_server */
	rcv_thread_id = msg.snd_thread_id();

	return rcv_thread_id;
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
	/* insert number of capabilities to be send into msgbuf */
	_snd_msg->buf[0] = _snd_msg->cap_count();
	/* IPC send operations */
	for(int i=0; i<50; i++)
		printf("%lu,", _snd_msg->buf[i]);
	printf("\n");
	__share_buf(_dst, _snd_msg);
	__send_caps(_snd_msg, _dst);

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
	/**
	 * wait for an incomming message
	 *  and set dst so it can be used in Ipc_server
	 */
	_dst.rcv_thread_id = __rec_buf(_rcv_msg);
	__rec_caps(_rcv_msg);

	Spartan::as_area_destroy(_rcv_msg->buf);

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

	/* send the request */
	Ipc_ostream::_send();

	/* ask for the reply */
//	__send_buf(Ipc_ostream::_dst, _rcv_msg, _obtain_utcb()->thread_id(),
//	           &Spartan::ipc_data_read);

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
	/* insert number of capabilities to be send into msgbuf */
	_snd_msg->buf[0] = _snd_msg->cap_count();
	/* wait for the question for the reply */
//	__rec_buf(_snd_msg, &Spartan::ipc_data_read_accept, IPC_M_DATA_READ);

	/* wait for the question to clone capabilities */
	for(addr_t i=0; i<Ipc_ostream::_snd_msg->cap_count(); i++) {
		Thread_utcb* my_utcb = _obtain_utcb();
		Ipc_message msg = my_utcb->msg_queue()->wait_for_call(
		                                        my_utcb->thread_id(),
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
