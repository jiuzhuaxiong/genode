#include <spartan/syscalls.h>
#include <base/native_types.h>
#include <base/printf.h>
#include <base/ipc_call.h>
#include <base/ipc_manager_thread.h>

using namespace Genode;

Ipc_call_queue::Ipc_call_queue()
: _call_count(0)
{
	for(int i=0; i<QUEUE_SIZE; i++) {
		_call_list[i] = 0;
		_used[i] = false;
	}
}

int
Ipc_call_queue::_get_first_free_slot()
{
	printf("Ipc_call_queue:\tlooking for first free slot\n");
	if(_call_count >= QUEUE_SIZE) {
		return FULL_IPC_QUEUE;
	}

	/* spot first unused position */
	int pos = 0;
	while(_used[pos] && pos<QUEUE_SIZE)
		pos++;

	if(pos+1 == QUEUE_SIZE)
		pos = FULL_IPC_QUEUE;

	return pos;
}

int
Ipc_call_queue::call_count(Native_thread_id rcv_thread_id)
{
	int count = 0;;

	Lock::Guard lock_guard(_queue_lock);
	for(int i=0; i<_call_count; i++)
		if(_call_list[i]->dest_thread_id() == rcv_thread_id)
			count++;

	return count;
}

int
Ipc_call_queue::insert_new(Ipc_call new_call)
{
	/* obtain free position in saving queue */
	printf("Ipc_call_queue:\tstarting to insert new call\n");

	Lock::Guard lock_guard(_queue_lock);
	int insert_pos = _get_first_free_slot();

	if(insert_pos >= 0) {
		printf("Ipc_call_queue:\t inserting into _call_queue at pos %i", insert_pos);
		/* insert call at obtained position */
		_call_queue[insert_pos] = new_call;
		/* mark obtained position as used */
		_used[insert_pos] = true;
		/* insert call in call list */
		printf("Ipc_call_queue:\t inserting into _call_list at pos %i", _call_count);
		_call_list[_call_count] = _call_queue + insert_pos;
		/* increase call count */
		_call_count++;
	}

	return insert_pos >= 0 ? 0 : insert_pos;
}

Ipc_call
Ipc_call_queue::get_first(Native_thread_id rcv_thread_id)
{
	Ipc_call	ret_call = Ipc_call();

	Lock::Guard lock_guard(_queue_lock);
	/* iterate through call list to find first matching call */
	for(int pos=0; pos<_call_count; pos++)
		/* check if the selected call matches */
		if((_call_list[pos]->dest_thread_id() == rcv_thread_id)) {
			/* calculate position in saving queue */
			int save_pos = _call_list[pos]-_call_queue;
			/* save call to be returned */
			ret_call = _call_queue[save_pos];
			/* mark position unused */
			_used[save_pos] = false;

			/* move all later dropped in calls one forward */
			for(int i=pos; i<_call_count; i++)
				_call_list[i] = _call_list[i+1];
			/* decrease call count */
			_call_count--;
			break;
		}

	return ret_call;
}

Ipc_call
Ipc_call_queue::get_last(void)
{
	Ipc_call	ret_call = Ipc_call();

	Lock::Guard lock_guard(_queue_lock);
	if(_call_count <= 0)
		return ret_call;

	int save_pos = _call_list[_call_count]-_call_queue;
	ret_call = _call_queue[save_pos];
	_used[save_pos] = false;
	_call_count--;

	return ret_call;
}

Ipc_manager_thread::~Ipc_manager_thread()
{
	Ipc_call del_call;
	/* Answer every pending call with error code TASK_KILLED */
	while((del_call = _call_queue.get_last()) != Ipc_call()) {
		Spartan::ipc_answer_0(del_call.callid(), TASK_KILLED);
	}
	/* TODO
	 * kill the thread if exist
	 */
}

bool
Ipc_manager_thread::_create()
{
	if(!_initialized && _thread_id == Spartan::INVALID_ID) {
		enum { STACK_SIZE = 16384 };
		static char stack[STACK_SIZE];
		printf("Ipc_manager_thread:\tinitializing manager thread!\n");

		_thread_id = Spartan::thread_create((void*)&Ipc_manager_thread::_wait_for_call, &stack, STACK_SIZE,
				"manager_thread");
		printf("Ipc_manager_thread:\t new thread has id %lu\n", _thread_id);
		if(_thread_id != Spartan::INVALID_ID)
			_initialized = true;
	}

	return _initialized;
}

void
Ipc_manager_thread::_wait_for_call()
{
	Genode::Native_ipc_callid	callid = 0;
	Genode::Native_ipc_call		call;

	while(1) {
		callid = Spartan::ipc_wait_for_call_timeout(&call, 0);

		printf("Ipc_manager_thread:\treceived incomming call\n"
			"\t\tIMETHOD=%lu, ARG1=%lu(destination task), "
			"ARG2=%lu(destination thread), ARG3=%lu(sending thread), "
			"ARG4=%lu, ARG5=%lu\n", IPC_GET_IMETHOD(call),
			IPC_GET_ARG1(call), IPC_GET_ARG2(call),
			IPC_GET_ARG3(call), IPC_GET_ARG4(call),
			IPC_GET_ARG5(call));

		printf("Ipc_manager_thread:\tnomnom\n");
		int ret;
		if((ret = _call_queue.insert_new(Ipc_call(callid, call))) != 0 ) {
			/* the call could not be inserted */
			printf("Ipc_manager_thread:\trejecting call\n");
			Spartan::ipc_answer_0(callid, ret);
		}
	}
}

addr_t
Ipc_manager_thread::get_call_count(Native_thread_id rcv_thread_id)
{
	return _call_queue.call_count(rcv_thread_id);
}

Ipc_call
Ipc_manager_thread::get_next_call(Native_thread_id rcv_thread_id)
{
	Ipc_call	call;

	_create();

	printf("Ipc_manager_thread:\t:get_next_call()\n");
	call = _call_queue.get_first(rcv_thread_id);

	return call;
}

Ipc_call
Ipc_manager_thread::wait_for_call(Native_thread_id rcv_thread_id)
{
	Ipc_call	call = get_next_call(rcv_thread_id);

	while(call.callid() == 0) {
		call = get_next_call(rcv_thread_id);
		Spartan::usleep(10000);
	}

	return call;
}

