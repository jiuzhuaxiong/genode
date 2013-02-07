#include <base/thread_utcb.h>
#include <base/ipc_manager.h>
#include <base/printf.h>

#include <spartan/errno.h>

//#include <base/printf.h>

using namespace Genode;


Thread_utcb::Thread_utcb()
: _waiting_for_ipc(false)
{
	set_thread_id();
}

Thread_utcb::~Thread_utcb()
{
	Ipc_message del_call;
	Ipc_message cmp_val = Ipc_message();
	/* Answer every pending call, which is not an answer, with error code */
	while(((del_call = _msg_queue.get_last()) != cmp_val)
	      && !(del_call.callid() & IPC_CALLID_ANSWERED)) {
		Spartan::ipc_answer_0(del_call.callid(), del_call.snd_task_id(), E__THREAD_KILLED);
	}
	Ipc_manager::singleton()->unregister_thread(this);
}


void
Thread_utcb::set_thread_id(bool is_main_thread)
{
	if(!is_main_thread)
		_thread_id = _thread_counter()->inc();
	else
		_thread_id = 0;
	_global_thread_id = Spartan::thread_get_id();

	Ipc_manager::singleton()->register_thread(this);
}


void
Thread_utcb::insert_msg(Ipc_message msg)
{
	/* if an invalid call is to be inserted (marking to take 
	 *  the governship of the Ipc_manager) unset _waiting_for_ipc
	 *  so no more invalid calls will get inserted until the 
	 *  Ipc_message_queue has been left and initiated again */
	if(!msg.is_valid())
		_waiting_for_ipc = false;

	_msg_queue.insert_new(msg);
}


Ipc_message
Thread_utcb::wait_for_call(addr_t imethod)
{
	Ipc_message call = _msg_queue.get_first_imethod(false, imethod);

	while(!call.is_valid()) {
		Ipc_manager::singleton()->get_call(_thread_id);
		_waiting_for_ipc = true;
		call = _msg_queue.get_first_imethod(true, imethod);
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


Ipc_message
Thread_utcb::wait_for_answer(Native_ipc_callid callid)
{
	Ipc_message answer = _msg_queue.get_first_answer_callid(false, callid);

	while(!answer.is_valid()) {
		Ipc_manager::singleton()->get_call(_thread_id);
		_waiting_for_ipc = true;
		answer = _msg_queue.get_first_answer_callid(true, callid);
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

