#include <spartan/syscalls.h>
#include <base/native_types.h>
#include <base/printf.h>
#include <base/ipc_call.h>
#include <base/ipc_manager.h>

using namespace Genode;

void _manager_thread_entry()
{
	static bool		initialized = false;

	if(!initialized) {
		initialized = true;
		Ipc_manager::singleton()->loop_answerbox();
	}
}


bool
Ipc_manager::_create()
{
	if(_thread_id == Spartan::INVALID_ID) {
		static char stack[STACK_SIZE];
		printf("Ipc_manager:\tinitializing manager thread!\n");

		_thread_id = Spartan::thread_create((void*)_manager_thread_entry, &stack, STACK_SIZE,
				"manager_thread");
		printf("Ipc_manager:\t new thread has id %lu\n", _thread_id);
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

Thread_utcb*
Ipc_manager::my_thread()
{
	int pos;
	Native_thread_id thread_id = Spartan::thread_get_id();

//	printf("Ipc_manager:\tlooking for thread with id %lu\n", thread_id);
	if((pos = _get_thread(thread_id)) < 0)
		return 0;

	return _threads[pos];
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
		Genode::Native_ipc_callid	callid = 0;
		Genode::Native_ipc_call		n_call;
		callid = Spartan::ipc_wait_for_call_timeout(&n_call, 0);

		printf("Ipc_manager:\treceived incomming call\n"
			"\t\tIMETHOD=%lu, ARG1=%lu(destination task), "
			"ARG2=%lu(destination thread), ARG3=%lu(sending thread), "
			"ARG4=%lu, ARG5=%lu\n", IPC_GET_IMETHOD(n_call),
			IPC_GET_ARG1(n_call), IPC_GET_ARG2(n_call),
			IPC_GET_ARG3(n_call), IPC_GET_ARG4(n_call),
			IPC_GET_ARG5(n_call));

		Ipc_call call = Ipc_call(callid, n_call);
		int thread_pos;
		if((thread_pos = _get_thread(call.dest_thread_id())) < 0) {
			/* there is no such thread */
			printf("Ipc_manager:\trejecting call\n");
			Spartan::ipc_answer_0(callid, -1);
			continue;
		}

		if(call.call_method() != IPC_M_DATA_READ) {
			int ret;
			if((ret = _threads[thread_pos]->insert_call(call)) < 0) {
				/* could not insert call */
				printf("Ipc_manager:\trejecting call\n");
				Spartan::ipc_answer_0(callid, ret);
			}
		}
		else
			while(!_threads[thread_pos]->insert_reply(call));
	}
}

bool
Ipc_manager::register_thread(Thread_utcb* new_thread)
{
	if(_thread_count >= MAX_THREAD_COUNT)
		return false;

	printf("Ipc_manager:\tregistering new thread with thread_id=%lu\n", new_thread->thread_id());
	Lock::Guard lock(_thread_lock);
	_threads[_thread_count++] = new_thread;
	return true;
}

void
Ipc_manager::unregister_thread()
{
	Native_thread_id thread_id = Spartan::thread_get_id();

	Lock::Guard lock(_thread_lock);
	int pos = _get_thread(thread_id);
	for(int i=pos; i<(_thread_count-1); i++)
		_threads[i] = _threads[i+1];
}

