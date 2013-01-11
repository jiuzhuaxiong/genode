
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
#include <base/ipc_call.h>
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
	                               Spartan::thread_get_id());
}

bool _receive_capability(Msgbuf_base *rcv_msg)
{
	Ipc_call call = Ipc_manager::singleton()->my_utcb()->wait_for_call(
	                 IPC_M_CONNECTION_CLONE);

	Ipc_destination dest = {call.target_thread_id(), call.cloned_phone()};
	Native_capability  cap = Native_capability(dest, call.capability_id());
	Spartan::ipc_answer_0(call.callid(), call.snd_thread_id(), 0);

	PDBG("_receive_capability:\tincomming phone=%i connecting to thread %lu", cap.dst().snd_phone, cap.dst().rcv_thread_id);

	return rcv_msg->cap_append(cap);
}


/*****************
 ** Ipc_ostream **
 *****************/

void Ipc_ostream::_send()
{
	Ipc_call          rpl_call;
	Native_ipc_callid snd_callid[Msgbuf_base::MAX_CAP_ARGS];
	/* insert number of capabilities to be send into msgbuf */
	_snd_msg->buf[0] = _snd_msg->cap_count();
	PDBG("_snd_msg->buf[0]=%i, _snd_msg->cap_count()=%lu", _snd_msg->buf[0], _snd_msg->cap_count());
	/* perform IPC send operation
	 *
	 * Check whether the phone_id is valid and send the message
	 */
	PDBG("sending via phone %i", _dst.dst().snd_phone);
	snd_callid[0] = Spartan::ipc_data_write(_dst.dst().snd_phone,
	                                     _snd_msg->buf, _snd_msg->size(),
	                                     _dst.dst().rcv_thread_id,
	                                     Spartan::thread_get_id());

	rpl_call = Ipc_manager::singleton()->my_utcb()->wait_for_reply(
	            snd_callid[0]);
	if(rpl_call.answer_code() != EOK) {
		PERR("ipc error in _send [ErrorCode: %lu]",
		     rpl_call.answer_code());
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
		rpl_call = Ipc_manager::singleton()->my_utcb()->wait_for_reply(
		            snd_callid[i]);
		if(rpl_call.answer_code() != EOK) {
			PERR("ipc error while sending capabilities "
			     "[ErrorCode: %lu]", rpl_call.answer_code());
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

void Ipc_istream::_wait()
{
	Ipc_call		call;
	addr_t			size;

	/*
	 * Wait for IPC message
	 */
	call = Ipc_manager::singleton()->my_utcb()->wait_for_call(IPC_M_DATA_WRITE);
	PDBG("got call with callid %lu\n", call.callid());
	if(call.method() != IPC_M_DATA_WRITE) {
		/* unknown sender */
		PDBG("Ipc_istream:\twrong call method (call.call_method()!=IPC_M_DATA_WRITE)!\n");
		Spartan::ipc_answer_0(call.callid(), call.snd_thread_id(), -1);
		return;
	}
	_rcv_msg->callid = call.callid();
	size = call.msg_size();

	/* Retrieve the message */
	/* TODO compare send message size with my own message size
	 * -> which policy should be implemented?
	 */
	Spartan::ipc_data_accept(call.callid(), _rcv_msg->buf, size,
	                         call.snd_thread_id());
//	PDBG("Ipc_istream:\twrite finalize returned %i\n", ret);

	/* set dst so it can be used in Ipc_server */
	_dst.rcv_thread_id = call.snd_thread_id();

	/* extract all retrieved capailities */
	PDBG("_rcv_msg->buf[0] = %i\n", _rcv_msg->buf[0]);
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

void Ipc_client::_call() { }

Ipc_client::Ipc_client(Native_capability const &srv, Msgbuf_base *snd_msg,
                       Msgbuf_base *rcv_msg)
: Ipc_istream(rcv_msg), Ipc_ostream(srv, snd_msg), _result(0) { }

void Ipc_server::_wait() { }

void Ipc_server::_reply() { }

void Ipc_server::_reply_wait() { }

Ipc_server::Ipc_server(Msgbuf_base *snd_msg, Msgbuf_base *rcv_msg)
: Ipc_istream(rcv_msg), Ipc_ostream(Native_capability(), snd_msg) { }
