#include <base/thread_utcb.h>
#include <base/ipc_manager.h>
#include <base/printf.h>

#include <spartan/errno.h>

//#include <base/printf.h>

using namespace Genode;


Thread_utcb::Thread_utcb()
: _waiting_for_ipc(false)
{
	_thread_id = _new_thread_id();
	_global_thread_id = Spartan::thread_get_id();

	Ipc_manager::singleton()->register_thread(this);
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


bool
Thread_utcb::is_waiting_for_ipc()
{
	return _msg_queue.is_waiting();
}

