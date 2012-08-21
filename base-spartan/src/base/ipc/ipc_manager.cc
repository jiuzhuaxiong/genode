/*
 * \brief  Implementation of Spartan-exclusive ipc dispatcher
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
#include <base/native_types.h>
#include <base/printf.h>
#include <base/ipc_call.h>
#include <base/ipc_manager.h>
#include <base/thread.h>

/* Spartan includes */
#include <spartan/syscalls.h>

using namespace Genode;


/**********************
 ** Helper functions **
 **********************/

void _manager_thread_entry()
{
	static bool		initialized = false;

	if(!initialized) {
		initialized = true;
		Ipc_manager::singleton()->loop_answerbox();
	}
}


/*****************
 ** Ipc Manager **
 *****************/

bool
Ipc_manager::_create()
{
	if(_thread_id == Spartan::INVALID_ID) {
		static char stack[STACK_SIZE];
		PDBG("initializing manager thread!\n");

		_thread_id = Spartan::thread_create((void*)_manager_thread_entry, &stack, STACK_SIZE,
		                                    "manager_thread");
		PDBG(" new thread has id %lu\n", _thread_id);
	}

	return _initialized;
}


int
Ipc_manager::_get_thread(Native_thread_id thread_id)
{
	Lock::Guard lock(_thread_lock);
	for(addr_t i=0; i<_thread_count; i++)
		if(_threads[i]->thread_id() == thread_id)
			return i;

	return -1;
}


int
Ipc_manager::_get_thread(Thread_utcb* utcb)
{
	Lock::Guard lock(_thread_lock);
	for(addr_t i=0; i<_thread_count; i++)
		if(_threads[i] == utcb)
			return i;

	return -1;
}


void
Ipc_manager::wait_for_calls()
{
	_create();
}


void
Ipc_manager::loop_answerbox()
{
	/* make shure there is only one instance of this function running */
	if(_initialized)
		return;
	else
		_initialized = true;

	/* loop forever and grab all incomming calls */
	while(1) {
		Genode::Native_ipc_callid callid = 0;
		Genode::Native_ipc_call   new_call;
		callid = Spartan::ipc_wait_for_call_timeout(&new_call, 0);

		PDBG("Ipc_manager:\treceived incomming call\n"
		     "\t\tIMETHOD=%lu, ARG1=%lu(destination task), "
		     "ARG2=%lu(destination thread), ARG3=%lu(sending thread), "
		     "ARG4=%lu, ARG5=%lu\n", IPC_GET_IMETHOD(new_call),
		     IPC_GET_ARG1(new_call), IPC_GET_ARG2(new_call),
		     IPC_GET_ARG3(new_call), IPC_GET_ARG4(new_call),
		     IPC_GET_ARG5(new_call));

		Ipc_call call = Ipc_call(callid, new_call);
		int thread_pos;
		if((thread_pos = _get_thread(call.dest_thread_id())) < 0) {
			/* there is no such thread */
			PDBG("Ipc_manager:\trejecting call\n");
			Spartan::ipc_answer_0(callid, -1);
			continue;
		}

		if(call.call_method() != IPC_M_DATA_READ) {
			try {
				_threads[thread_pos]->insert_call(call);
			} catch (Ipc_call_queue::Overflow) {
				/* could not insert call */
				PDBG("Ipc_manager:\trejecting call\n");
				Spartan::ipc_answer_0(callid, -1);
			}
		}
		else
			while(!_threads[thread_pos]->insert_reply(call));
	}
}


bool
Ipc_manager::register_thread(Thread_utcb* utcb)
{
	if(_thread_count >= MAX_THREAD_COUNT)
		return false;

	int pos = _get_thread(utcb);
	if (pos < 0) {
		PDBG("registering new thread with thread_id=%lu "
		     "while utcb=%lu\n", utcb->thread_id(), utcb);
		Lock::Guard lock(_thread_lock);
		_threads[_thread_count++] = utcb;
	}
	return true;
}


void
Ipc_manager::unregister_thread(Thread_utcb* utcb)
{
	Native_thread_id thread_id = utcb->thread_id();

	int pos = _get_thread(thread_id);
	if (pos < 0)
		return;

	Lock::Guard lock(_thread_lock);
	for(int i=pos; i<(_thread_count-1); i++)
		_threads[i] = _threads[i+1];
}

