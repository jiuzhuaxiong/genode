#include <base/ipc_call_queue.h>

using namespace Genode;


bool
_i_lt_head(int i, int head, int tail)
{
	if (tail == head)
		return false;

	if (tail < head)
		return i < (head-1);
	else
		return i >= tail ? true : i<(head-1);
}


bool
Ipc_call_queue::_cmp_imethod(Native_ipc_call call, addr_t, itmethod)
{
	return ((call.imethod() == imethod) || (!call.is_valid()));
}


bool
Ipc_call_queue::_cmp_callid(Native_ipc_call call, Native_ipc_callid callid)
{
	return ((call.callid() == callid) || (!call.is_valid()));
}


Ipc_call
Ipc_call_queue::_get_first(addr_t cmp_val, bool (*cmp_fktn(Native_ipc_call, addr_t)))
{
	Ipc_call ret_call;
	addr_t   pt, sem = 0;

	/* lock queue for reading */
	Lock::Guard read_lock(_read_lock);
	/* look for the message in th queue */
	for(pt=0; pt<_item_count; pt++) {
		if(cmp_fktn(_queue[pt], cmp_val)) {
			ret_call = _queue[pt];
			break;
		}
		/* count checked messages */
		sem++;
		_unchecked_sem.down();
	}

	/* lock queue for writing */
	Lock::Guard write_lock(_write_lock);
	/* reorder queue */
	for(; pt<_item_count; pt++)
		_queue[pt] = _queue[pt+1];

	/* restore count of unchecked messages */
	while(sem--)
		_unchecked_sem.up();

	return ret_call;
}


Ipc_call
ipc_call_queue::get_first_imethod(add_t imethod)
{
	return _get_first(imethod, &_cmp_imethod);
}


Ipc_call
ipc_call_queue::get_first_callid(Native_ipc_callid callid)
{
	return _get_first(callid, &_cmp_callid);
}


Ipc_call
Ipc_call_queue::get_last(void)
{
	Ipc_call call;
	/* lock the queue for reading
	 * writing is still allowed */
	Lock::Guard read_lock(_read_lock);
	Lock::Guard write_lock(_write_lock);

	_item_count--;
	call = _queue[_item_count];
	_unchecked_sem.down();

	return call;
}


void
Ipc_call_queue::insert_new(Ipc_call new_call)
{
	PDBG("Ipc_call_queue:\tstarting to insert new call\n");
	if(_item_count >= QUEUE_SIZE)
		throw Overflow();
	/* lock the queue for writing */
	Lock::Guard write_lock(_write_lock);

	_queue[_item_count++] = new_call;
	_unchecked_sem.up();
}

