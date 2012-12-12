#include <spartan/ipc.h>
#include <base/native_types.h>
#include <base/ipc_call.h>
#include <base/ipc_manager.h>
#include <base/thread.h>

using namespace Genode;


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
Ipc_manager::_wait_for_calls()
{
	Native_thread_id my_thread_id = Spartan::thread_get_id();

	/* loop and grab all incomming calls as long as there is no call for me */
	while(1) {
		Genode::Native_ipc_call		n_call;
		n_call.callid = Spartan::ipc_wait_for_call_timeout(&n_call, 0);

		PDBG("Ipc_manager:\treceived incomming call\n"
		     "\t\tIMETHOD=%lu, ARG1=%lu(destination task), "
		     "ARG2=%lu(destination thread), ARG3=%lu(sending thread), "
		     "ARG4=%lu, ARG5=%lu\n", IPC_GET_IMETHOD(n_call),
		     IPC_GET_ARG1(n_call), IPC_GET_ARG2(n_call),
		     IPC_GET_ARG3(n_call), IPC_GET_ARG4(n_call),
		     IPC_GET_ARG5(n_call));

		Ipc_call call = Ipc_call(n_call);
		if(!call.is_valid()) {
			continue;
		}

		int thread_pos;
		if((thread_pos = _get_thread(call.dest_thread_id())) < 0) {
			/* there is no such thread */
			PDBG("Ipc_manager:\trejecting call\n");
			Spartan::ipc_answer_0(call.callid(), call.snd_thread_id(),
			                      E__IPC_NO_SUCH_DESTINATION);
			continue;
		}

		if(call.callid() & call.callid & IPC_CALLID_ANSWERED) {
			/* ipc massage is actually an answer */
			_threads[thread_pos]->insert_reply(call);
		}
		else {
			try {
				_threads[thread_pos]->insert_call(call);
			} catch (Ipc_call_queue::Overflow) {
				/* could not insert call */
				PDBG("Ipc_manager:\trejecting call\n");
				Spartan::ipc_answer_0(call.callid(), call.snd_thread_id(),
				                      E__IPC_CALL_QUEUE_FULL);
			}
		}

		if(call.dest_thread_id() == my_thread_id) {
			/* mark the govenor as free */
			_governor = GOV_FREE;
			/**
			 * wake up the first waiting threadi
			 *  to take over the government
			 *  by sending an invalid ipc call
			 */
			if(_thread_count > 0) {
				_threads[0]->insert_call(Ipc_call());
			}
			return;
		}
	}
}


void
Ipc_manager::get_call()
{
	if(Genode::cmpxchg(&_governor, GOV_FREE, GOV_TAKEN)) {
		_wait_for_call();
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
		PDBG("Ipc_manager:\tregistering new thread with thread_id=%lu while utcb=%lu\n", utcb->thread_id(), utcb);
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

