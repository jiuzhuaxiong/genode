/*
 * \brief  Spartan-specific threads UTCB implementation
 * \author Tobias Börtitz
 * \date   2012-08-14
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/thread_utcb.h>
#include <base/ipc_call.h>
#include <base/ipc_manager.h>

/* Spartan includes */
#include <base/printf.h>

using namespace Genode;


Thread_utcb::~Thread_utcb()
{
	Ipc_call del_call;
	/* Answer every pending call with error code THREAD_KILLED */
	while((del_call = _call_queue.get_last()) != Ipc_call()) {
		/* TODO
		 * maybe it would be better to use HANGUP as returncode? */
		Spartan::ipc_answer_0(del_call.callid(), THREAD_KILLED);
	}
	Ipc_manager::singleton()->unregister_thread(this);
}


void
Thread_utcb::set_thread_id(Native_thread_id tid)
{
	_thread_id = tid;
	Ipc_manager::singleton()->register_thread(this);
}


void
Thread_utcb::insert_call(Ipc_call call)
{
	_call_queue.insert_new(call);
}


Ipc_call
Thread_utcb::get_next_call(addr_t imethod)
{
	Ipc_call	call;

	Ipc_manager::singleton()->wait_for_calls();

	call = _call_queue.get_first(imethod);

	return call;
}


Ipc_call
Thread_utcb::wait_for_call(addr_t imethod)
{
	Ipc_call	call = get_next_call(imethod);

	while((call.callid() == 0)) {
		call = get_next_call(imethod);
	}

	return call;
}


bool
Thread_utcb::insert_reply(Ipc_call call)
{
	if(_answer_sem.cnt() > 0)
		return false;

	Lock::Guard lock(_answer_lock);
	_answer_sem.up();
	_ipc_answer = call;

	return true;
}


Ipc_call
Thread_utcb::get_reply()
{
	Lock::Guard lock(_answer_lock);

	_answer_sem.down();
	Ipc_call answer = _ipc_answer;

	return answer;
}

