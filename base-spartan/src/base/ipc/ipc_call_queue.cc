#include <base/printf.h>

#include <base/ipc_call_queue.h>

using namespace Genode;

/********************
 * helper functions *
 ********************/

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


/* compare an imethod with the imethod of a call */
bool
_cmp_imethod(Ipc_call call, addr_t imethod)
{
	return (!(call.callid() & IPC_CALLID_ANSWERED) //the call is not an answer
	        && ((call.method() == imethod) //the call has the imethod we are looking for
	         || (!call.is_valid()) //we are advised to take over government of the ipc_manager
	         || (imethod == 0))); //we are not looking for a specific imethod
}


/* check whether a call is an answer to the specified callid */
bool
_cmp_reply_callid(Ipc_call call, Native_ipc_callid callid)
{
	return (((call.callid() & IPC_CALLID_ANSWERED) //the call is an answer
	         && (((call.callid() | IPC_CALLID_ANSWERED) == callid) //the call is the answer to the call we are looking for
	          || callid == 0)) //we are not looking for a specific answer
	        || (!call.is_valid())); //we are advised to take over government of the ipc_manager
}

/*********************
 * private functions *
 *********************/

/* get first occurence of a specific call from th queue
 *  comparison is defined through a function pointer */
Ipc_call
Ipc_call_queue::_get_first(bool blocking, addr_t cmp_val,
                           bool (*cmp_fktn)(Ipc_call, addr_t))
{
	Ipc_call ret_call;
	addr_t   pt = 0, sem = 0;

	/* lock queue for reading */
	Lock::Guard read_lock(_read_lock);
	/* look for the message in th queue */
//	PDBG("itemcount = %lu", _item_count);
	for(pt=0; pt<_item_count; pt++) {
//		PDBG("call.callid() & IPC_CALLID_ANSWERED returns %i & call.is_valid() returns %i", (_queue[pt].callid() & IPC_CALLID_ANSWERED), _queue[pt].is_valid());
		/* did we find the requested call? */
		if(cmp_fktn(_queue[pt], cmp_val)) {
			ret_call = _queue[pt];
			break;
		}

		/* the requested call is not in the queue
		 *  and we shall not block */
		if(!blocking && (_sem.cnt() < 1)) {
			PDBG("NON-blocking return");
			return ret_call;
		}

		if(_sem.cnt() < 1)
			PDBG("BLOCK");
		/* count checked messages */
		sem++;
		_sem.down();
	}

	/* lock queue for writing */
	Lock::Guard write_lock(_write_lock);
	/* reorder queue */
	if(_item_count > 0)
		_item_count--;
	for(; pt<_item_count; pt++)
		_queue[pt] = _queue[pt+1];

	/* restore count of unchecked messages */
	while(sem--)
		_sem.up();

	return ret_call;
}

/********************
 * public functions *
 ********************/

/* get the first call with the specified imethod */
Ipc_call
Ipc_call_queue::get_first_imethod(bool blocking, addr_t imethod)
{
	return _get_first(blocking, imethod, &_cmp_imethod);
}


/* get the first reply to the sepcified callid */
Ipc_call
Ipc_call_queue::get_first_reply_callid(bool blocking, Native_ipc_callid callid)
{
	return _get_first(blocking, callid, &_cmp_reply_callid);
}


/* get the last call/answer without any checking */
Ipc_call
Ipc_call_queue::get_last(void)
{
	Ipc_call call;
	/* lock the queue for reading
	 * writing is still allowed */
	Lock::Guard read_lock(_read_lock);
	Lock::Guard write_lock(_write_lock);

	if(_item_count > 0) {
		_item_count--;
		call = _queue[_item_count];
		_sem.down();
	}

	return call;
}


/* inserts a new call into the queue */
void
Ipc_call_queue::insert_new(Ipc_call new_call)
{
	PDBG("Ipc_call_queue:\tstarting to insert new call\n");
	if(_item_count >= QUEUE_SIZE)
		throw Overflow();
	/* lock the queue for writing */
	Lock::Guard write_lock(_write_lock);

	_queue[_item_count++] = new_call;
	_sem.up();
}


/* checks whether or not the Ipc_call_queue is waiting for calls */
bool
Ipc_call_queue::is_waiting()
{
	return (_sem.cnt() < 0);
}

