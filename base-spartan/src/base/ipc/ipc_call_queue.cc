#include <base/ipc_call_queue.h>
#include <base/printf.h>

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
	Lock::Guard lock_guard(_queue_lock);

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
Ipc_call_queue::insert_new(Ipc_call new_call)
{
	/* obtain free position in saving queue */
	printf("Ipc_call_queue:\tstarting to insert new call\n");
	int insert_pos = _get_first_free_slot();

	if(insert_pos >= 0) {
		Lock::Guard lock_guard(_queue_lock);

		/* insert call at obtained position */
		_call_queue[insert_pos] = new_call;
		/* mark obtained position as used */
		_used[insert_pos] = true;
		/* insert call in call list */
		_call_list[_call_count] = _call_queue + insert_pos;
		/* increase call count */
		_call_count++;
		printf("Ipc_call_queue:\t inserted new _call_list. callcount=%i\n", _call_count);
	}

	return insert_pos >= 0 ? 0 : insert_pos;
}

Ipc_call
Ipc_call_queue::get_first(addr_t imethod)
{
	Ipc_call	ret_call = Ipc_call();

	Lock::Guard lock_guard(_queue_lock);
	/* iterate through call list to find first matching call */
	for(int pos=0; pos<_call_count; pos++)
	/* check if the selected call matches */
	if((_call_list[pos]->call_method() == imethod) || (imethod == 0)) {
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
