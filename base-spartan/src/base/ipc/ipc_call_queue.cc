#include <base/printf.h>

#include <base/ipc_call_queue.h>

#include <spartan/syscalls.h>

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

//	PDBG("FOO?");
	/* lock queue for reading */
	Lock::Guard read_lock(_read_lock);
//	PDBG("BAR?!");

	do {
		/* break the loop if the requested message could not
		 *  be found and the queue request is non-blocking */
		PDBG("_sem.cnt() = %i", _sem.cnt());
		if(!blocking && (_sem.cnt() < 1)) {
			PDBG("NON-blocking return thread %lu", Spartan::thread_get_id());
			break;
		}

		if(_sem.cnt() < 1)
			PDBG("BLOCKING thread %lu while _sem.cnt()=%i", Spartan::thread_get_id(), _sem.cnt());
		/* decrease _sem before looking at the current position of the queue
		 * this ensures, that the queue will block the thread whenever there is
		 *  1) no message in the queue (on fresh entry of this function)
		 *  2) no unread message in the queuei (else)
		 *  since _sem holds the amount of all messages on fresh entry and 
		 *  otherwise the amount of unchecked messages */
		_sem.down();
		if(_sem.cnt() < 1)
			PDBG("AWAKEN thread %lu", Spartan::thread_get_id());
		/* did we find the requested call? */
		if(cmp_fktn(_queue[pt], cmp_val)) {
			PDBG("requested message found at position %lu", pt);
			ret_call = _queue[pt];
			_item_count--;
			break;
		}
		PDBG("requested message not found at position %lu", pt);
	} while(pt++, pt<_item_count);
	/* safe pt to re-increase _sem
	 * pt is exact the amount pf read messages */
	sem = pt;

	/* lock queue for writing */
	Lock::Guard write_lock(_write_lock);
	/* reorder queue if we took something out */
	if(blocking || _sem.cnt() > 0)
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
	PDBG("starting to insert new call in thread %lu. Current _item_count=%lu, _sem.cnt()=%lu",
	     Spartan::thread_get_id(), _item_count, _sem.cnt());
	if(_item_count >= QUEUE_SIZE)
		throw Overflow();
	/* lock the queue for writing */
	Lock::Guard write_lock(_write_lock);

	_queue[_item_count++] = new_call;
	_sem.up();
	PDBG("new _item_count=%lu, _sem.cnt()=%lu", _item_count, _sem.cnt());
}


/* checks whether or not the Ipc_call_queue is waiting for calls */
bool
Ipc_call_queue::is_waiting()
{
	PDBG("_sem.cnt() on thread %lu = %i", Spartan::thread_get_id(), _sem.cnt());
	return (_sem.cnt() < 0);
}

