/* general Genode includes */
#include <base/printf.h>
#include <cpu/atomic.h>

/* SPARTAN sepcefic Genode includes */
#include <base/native_types.h>
#include <base/ipc_message.h>
#include <base/ipc_manager.h>
#include <base/thread.h>

/* SPARTAN includes */
#include <spartan/ipc.h>
#include <spartan/errno.h>

using namespace Genode;

template<int QUEUE_SIZE>
void Thread_buffer<QUEUE_SIZE>::add(Thread_utcb* utcb)
{
	if(_thread_count >= QUEUE_SIZE)
		throw Overflow();

	int pos = exists_utcbpt(utcb);
	Lock::Guard lock(_thread_lock);
//	PDBG("%lu: adding thread %lu(%lu), pos=%i", this, utcb->thread_id(), utcb, pos);
	if(pos < 0) {
		_threads[_thread_count++] = utcb;
//		PDBG("new _thread_count=%i", _thread_count);
	}
//	for(int i=0; i<_thread_count; i++)
//		PDBG("_threads[%i](%lu)->thread_id()=%lu, waiting=%i", i, _threads[i], _threads[i]->thread_id(), _threads[i]->is_waiting_for_ipc());
}

/* look for a threads specified by its id */
template<int QUEUE_SIZE>
Thread_utcb* Thread_buffer<QUEUE_SIZE>::exists_threadid(Native_thread_id thread_id)
{
	Lock::Guard lock(_thread_lock);

	for(int i=0; i<_thread_count; i++)
		if(_threads[i]->thread_id() == thread_id)
			return _threads[i];
	return 0;
}


/* look for a thread specified by th pointer to its utcb */
template<int QUEUE_SIZE>
int Thread_buffer<QUEUE_SIZE>::exists_utcbpt(Thread_utcb* utcb)
{
	Lock::Guard lock(_thread_lock);

	for(int i=0; i<_thread_count; i++)
		if(_threads[i] == utcb)
			return i;

	return -1;
}

/* sends a certain message to the first registered threads in the queue which is
 *  waiting for IPC except for the one specified by thread_id */
template<int QUEUE_SIZE>
void Thread_buffer<QUEUE_SIZE>::message_first_waiting(Ipc_message msg,
                                                      Native_thread_id thread_id)
{
	Lock::Guard lock(_thread_lock);
	for(int i=0; i<_thread_count; i++) {
//		PDBG("%lu(%lu): waiting for ipc = %i", _threads[i]->thread_id(), _threads[i], _threads[i]->is_waiting_for_ipc());
		if(_threads[i]->msg_queue()->is_waiting()
		   && !(_threads[i]->thread_id() == thread_id)) {
			_threads[i]->msg_queue()->insert(msg);
			return;
		}
	}
}

template<int QUEUE_SIZE>
void Thread_buffer<QUEUE_SIZE>::del(Thread_utcb* utcb)
{
	int pos = exists_utcbpt(utcb);
	if(pos < 0)
		return;

	Lock::Guard lock(_thread_lock);
	for(int i=pos; (i+1)<_thread_count; i++)
		_threads[i] = _threads[i+1];
	_thread_count--;
}


/* wait for incomming calls until an incomming call is destined to the
 *  executing thread */
void
Ipc_manager::_wait_for_calls()
{
//	Native_thread_id my_thread_id = Spartan::thread_get_id();
		//Thread_base::myself()->utcb()->thread_id();

	/* loop and grab all incomming calls as long as there is no call for me */
	while(1) {
		Native_ipc_call n_call;
		/* wait for incoming calls */
		n_call = Spartan::ipc_wait_for_call_timeout(0);

		PDBG("Ipc_manager: received incomming call with callid=%lu\n"
		     "\t\tIMETHOD=%lu, ARG1=%lu, "
		     "ARG2=%lu, ARG3=%lu(sending thread), "
		     "ARG4=%lu(destination thread), ARG5=%lu\n"
		     "\t\tmy own thrad_id is %lu",
		     n_call.callid, IPC_GET_IMETHOD(n_call), 
		     IPC_GET_ARG1(n_call), IPC_GET_ARG2(n_call), 
		     IPC_GET_ARG3(n_call), IPC_GET_ARG4(n_call), 
		     IPC_GET_ARG5(n_call), _governor);

		/* check whether the incomming call is valid. if not, desmiss it */
		Ipc_message msg = Ipc_message(n_call);
		if(!msg.is_valid()) {
			continue;
		}

		/* look up the destined thread */
		Thread_utcb* dest_thread = _threads.exists_threadid(msg.dst_thread_id());
		if(dest_thread == 0) {
			/* there is no such thread */
			PDBG("Ipc_manager:\trejecting call because no such"
			     " requested thread found\n");
			Spartan::ipc_answer_0(msg.callid(), msg.snd_thread_id(),
			                      E__IPC_DESTINATION_UNKNOWN);
			continue;
		}

		/* insert the received call into the threads call queue */
		bool insert_success = false;
		try {
			insert_success = dest_thread->msg_queue()->insert(msg);
		} catch (Ipc_message_queue::Overflow) {
			/* could not insert call */
			PDBG("Ipc_manager:\trejecting call because of full"
			     " call queue\n");
			Spartan::ipc_answer_0(msg.callid(),
			                      msg.snd_thread_id(),
			                      E__IPC_CALL_QUEUE_FULL);
		}

		/**
		 * handle the case the destined thread is the current govenor thread
		 *  and the just inserted message is the one desired by the thread */
		if(insert_success
		   && msg.dst_thread_id() == _governor) {
			PDBG("thread %lu laying down governorship | insert_success=%i", _governor, insert_success);
			/* mark the govenor as free */
			_governor = GOV_FREE;
			/**
			 * wake up the last waiting thread
			 * (which is not the current thread)
			 *  to take over the government
			 *  by sending an invalid ipc call
			 */
			_threads.message_first_waiting(Ipc_message(), _governor);
			PDBG("ONMON");

			return;
		}
	}
}


bool
Ipc_manager::get_call(Native_thread_id thread_id)
{
	if(cmpxchg((int*)&_governor, GOV_FREE, thread_id)) {
		PDBG("new governor is thread with id %lu",
		     Spartan::thread_get_id());
		_wait_for_calls();

		return true;
	}

	return false;
}

/*
Native_utcb*
Ipc_manager::my_utcb()
{
	return 0;
}
*/

void
Ipc_manager::register_thread(Thread_utcb* utcb)
{
	Lock::Guard lock(_thread_lock);
//	Thread_utcb* t = _threads.exists_threadid(6);
//	if(t)
//		PDBG("%lu: is thread 6 waiting?: %i", Spartan::thread_get_id(), t->is_waiting_for_ipc());
	_threads.add(utcb);
}


void
Ipc_manager::unregister_thread(Thread_utcb* utcb)
{
	Lock::Guard lock(_thread_lock);
	_threads.del(utcb);
}

