#include <base/thread_utcb.h>
#include <base/ipc_call.h>
#include <base/ipc_manager.h>

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
	if(_answer_used)
		return false;

	Lock::Guard lock(_answer_lock);
	_ipc_answer = call;
	_answer_used = true;

	return true;
}

Ipc_call
Thread_utcb::get_reply()
{
	if(!_answer_used)
		return Ipc_call();

	Lock::Guard lock(_answer_lock);
	Ipc_call answer = _ipc_answer;
	_answer_used = false;

	return answer;
}
