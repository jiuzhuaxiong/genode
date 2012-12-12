#include <base/thread_utcb.h>
#include <base/ipc_call.h>
#include <base/ipc_manager.h>

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

	call = _call_queue.get_first_imethod(imethod);

	return call;
}


Ipc_call
Thread_utcb::wait_for_call(addr_t imethod)
{
	Ipc_call	call;

	while(!call.is_valid()) {
		Ipc_manager::singleton()->get_call();
		call = get_next_call(imethod);
		/**
		 * if the returned call is invalid the
		 *  government of the ipc_manager of another
		 *  thread has ended and should be acquired
		 *  by someone
		 */
	}
	return call;
}


void
Thread_utcb::insert_reply(Ipc_call call)
{
	_answer_queue.insert_new(call);
}


Ipc_call
Thread_utcb::get_next_answer(Native_ipc_callid callid)
{
	Ipc_call answer;

	answer = _call_queue.get_first_callid(callid);

	return answer;
}


Ipc_call
Thread_utcb::wait_for_call(Native_ipc_callid callid)
{
	Ipc_call answer;

	while(!answer.is_valid()) {
		Ipc_manager::singleton()->get_call();
		answer = get_next_answer(callid);
		/**
		 * if the returned answer is invalid the
		 *  government of the ipc_manager of another
		 *  thread has ended and should be acquired
		 *  by someone
		 */
	}
	return answer;
}

