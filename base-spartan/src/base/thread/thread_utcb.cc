#include <base/thread_utcb.h>
#include <base/ipc_manager.h>

#include <spartan/errno.h>

//#include <base/printf.h>

using namespace Genode;


Thread_utcb::~Thread_utcb()
{
	Ipc_call del_call;
	Ipc_call cmp_val = Ipc_call();
	/* Answer every pending call, which is not an answer, with error code */
	while(((del_call = _call_queue.get_last()) != cmp_val)
	      && !(del_call.callid() & IPC_CALLID_ANSWERED)) {
		Spartan::ipc_answer_0(del_call.callid(), del_call.snd_task_id(), E__THREAD_KILLED);
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
Thread_utcb::wait_for_call(addr_t imethod)
{
	_waiting_for_ipc = true;
	Ipc_call call = _call_queue.get_first_imethod(false, imethod);

	while(!call.is_valid()) {
		PDBG("%lu(%lu): is waiting = %i", Spartan::thread_get_id(), this, _waiting_for_ipc);
		Ipc_manager::singleton()->get_call();
		call = _call_queue.get_first_imethod(true, imethod);
		/**
		 * if the returned call is invalid the
		 *  government of the ipc_manager of another
		 *  thread has ended and should be acquired
		 *  by someone
		 */
	}
	_waiting_for_ipc = false;
	return call;
}


Ipc_call
Thread_utcb::wait_for_reply(Native_ipc_callid callid)
{
	_waiting_for_ipc = true;
	Ipc_call answer = _call_queue.get_first_reply_callid(false, callid);

	while(!answer.is_valid()) {
		PDBG("%lu(%lu): is waiting = %i", Spartan::thread_get_id(), this, _waiting_for_ipc);
		Ipc_manager::singleton()->get_call();
		answer = _call_queue.get_first_reply_callid(true, callid);
		/**
		 * if the returned answer is invalid the
		 *  government of the ipc_manager of another
		 *  thread has ended and should be acquired
		 *  by someone
		 */
	}
	_waiting_for_ipc = false;
	return answer;
}

bool
Thread_utcb::is_waiting_for_ipc()
{
	return _waiting_for_ipc;
}

