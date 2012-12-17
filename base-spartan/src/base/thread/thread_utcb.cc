#include <base/thread_utcb.h>
#include <base/ipc_manager.h>

#include <spartan/errno.h>

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
	Ipc_call call = _call_queue.get_first_imethod(false, imethod);

	while(!call.is_valid()) {
		Ipc_manager::singleton()->get_call();
		call = _call_queue.get_first_imethod(true, imethod);
		/**
		 * if the returned call is invalid the
		 *  government of the ipc_manager of another
		 *  thread has ended and should be acquired
		 *  by someone
		 */
	}
	return call;
}


Ipc_call
Thread_utcb::wait_for_reply(Native_ipc_callid callid)
{
	Ipc_call answer = _call_queue.get_first_reply_callid(false, callid);

	while(!answer.is_valid()) {
		Ipc_manager::singleton()->get_call();
		answer = _call_queue.get_first_reply_callid(true, callid);
		/**
		 * if the returned answer is invalid the
		 *  government of the ipc_manager of another
		 *  thread has ended and should be acquired
		 *  by someone
		 */
	}
	return answer;
}


bool
Thread_utcb::is_waiting()
{
	return _call_queue.is_waiting();
}

