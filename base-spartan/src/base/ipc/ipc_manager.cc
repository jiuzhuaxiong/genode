/* general Genode includes */
#include <base/printf.h>
#include <cpu/atomic.h>

/* SPARTAN sepcefic Genode includes */
#include <base/native_types.h>
#include <base/ipc_call.h>
#include <base/ipc_manager.h>
#include <base/thread.h>

/* SPARTAN includes */
#include <spartan/ipc.h>
#include <spartan/errno.h>

using namespace Genode;

/*******************
 * private methods *
 ******************/

/* look for a threads specified by its id */
int
Ipc_manager::_get_thread(Native_thread_id thread_id)
{
	Lock::Guard lock(_thread_lock);
	for(addr_t i=0; i<_thread_count; i++)
		if(_threads[i]->thread_id() == thread_id)
			return i;
	return -1;
}


/* look for a thread specified by th pointer to its utcb */
int
Ipc_manager::_get_thread(Thread_utcb* utcb)
{
	Lock::Guard lock(_thread_lock);
	for(addr_t i=0; i<_thread_count; i++)
		if(_threads[i] == utcb)
			return i;

	return -1;
}


/* wait for incomming calls until an incomming call is destined to the
 *  executing thread */
void
Ipc_manager::_wait_for_calls()
{
	Native_thread_id my_thread_id = Spartan::thread_get_id();

	/* loop and grab all incomming calls as long as there is no call for me */
	while(1) {
		Native_ipc_call n_call;
		/* wait for incoming calls */
		n_call = Spartan::ipc_wait_for_call_timeout(0);

		PDBG("Ipc_manager: received incomming call\n"
		     "\t\tIMETHOD=%lu, ARG1=%lu, "
		     "ARG2=%lu, ARG3=%lu(sending thread), "
		     "ARG4=%lu(destination thread), ARG5=%lu",
		     IPC_GET_IMETHOD(n_call), IPC_GET_ARG1(n_call),
		     IPC_GET_ARG2(n_call), IPC_GET_ARG3(n_call),
		     IPC_GET_ARG4(n_call), IPC_GET_ARG5(n_call));

		/* check whether the incomming call is valid. if not, desmiss it */
		Ipc_call call = Ipc_call(n_call);
		if(!call.is_valid()) {
			continue;
		}

		/* look up the destined thread */
		int thread_pos;
		if((thread_pos = _get_thread(call.dest_thread_id())) < 0) {
			/* there is no such thread */
			PDBG("Ipc_manager:\trejecting call because no such"
			     " requested thread found\n");
			Spartan::ipc_answer_0(call.callid(), call.snd_thread_id(),
			                      E__IPC_DESTINATION_UNKNOWN);
			continue;
		}

		/* insert the received call into the threads call queue */
		try {
			_threads[thread_pos]->insert_call(call);
		} catch (Ipc_call_queue::Overflow) {
			/* could not insert call */
			PDBG("Ipc_manager:\trejecting call because of full"
			     " call queue\n");
			Spartan::ipc_answer_0(call.callid(),
			                      call.snd_thread_id(),
			                      E__IPC_CALL_QUEUE_FULL);
		}

		/* handle the case the destined thread is the current govenor thread */
		if(call.dest_thread_id() == my_thread_id) {
//			PDBG("laying down governorship");
			/* mark the govenor as free */
			_governor = GOV_FREE;
			/**
			 * wake up the first waiting thread
			 *  to take over the government
			 *  by sending an invalid ipc call
			 */
			for(addr_t i=0; i<_thread_count; i++) {
				if((_threads[i]->thread_id() != my_thread_id)
				   && _threads[i]->is_waiting()) {
//					PDBG("inserting fake ipc call");
					_threads[i]->insert_call(Ipc_call());
					break;
				}
			}
			return;
		}
	}
}


void
Ipc_manager::get_call()
{
	if(cmpxchg(&_governor, GOV_FREE, GOV_TAKEN)) {
		_wait_for_calls();
	}
}


bool
Ipc_manager::register_thread(Thread_utcb* utcb)
{
	if(_thread_count >= MAX_THREAD_COUNT)
		return false;

	int pos = _get_thread(utcb);
	if (pos < 0) {
		/* utcb is not registered already, so register it */
		PDBG("registering new thread with thread_id=%lu while"
		     " utcb=%lu", utcb->thread_id(), utcb);
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

