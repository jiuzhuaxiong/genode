#include <base/printf.h>
#include <base/thread.h>

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
	return (!call.is_answer()
	        && (call.method() == imethod
	         || imethod == 0));
/*
	return (!(call.callid() & IPC_CALLID_ANSWERED) //the call is not an answer
	        && ((call.method() == imethod) //the call has the imethod we are looking for
	         || (!call.is_valid()) //we are advised to take over government of the ipc_manager
	         || (imethod == 0))); //we are not looking for a specific imethod
*/
}


/* check whether a call is an answer to the specified callid */
bool
_cmp_reply_callid(Ipc_call call, Native_ipc_callid callid)
{
//	PDBG("is %lu answer to %lu?: %i", call.callid(), callid, call.is_answer_to(callid));
	return (call.is_answer_to(callid)
	        || callid == 0);
/*
	return ((call.callid() & IPC_CALLID_ANSWERED) //the call is an answer
	         && (((call.callid() | IPC_CALLID_ANSWERED) == callid) //the call is the answer to the call we are looking for
	          || callid == 0)); //we are not looking for a specific answer
	        || (!call.is_valid())); //we are advised to take over government of the ipc_manager
*/
}

/*********************
 * private functions *
 *********************/

/**
 * remove the element at the given position from the queue.
 * make shure to safe the element in question before calling this function!
 */
void
Ipc_call_queue::_remove_from_queue(addr_t pos)
{
	/* lock queue for writing */
	Lock::Guard write_lock(_write_lock);
	/* reorder queue removing the element in question */
	for(; pos<(_item_count-1); pos++)
		_queue[pos] = _queue[pos+1];
	/* reduce the _item_count to its new value */
	_item_count--;
}

/**
 * get first occurence of a specific call from th queue
 *  comparison is defined through a function pointer
 */
Ipc_call
Ipc_call_queue::_get_first(bool blocking, addr_t cmp_val,
                           bool (*cmp_fktn)(Ipc_call, addr_t))
{
	Ipc_call ret_call;
	addr_t   pt = 0;
	bool     do_block = blocking;

	/* lock queue for reading */
	Lock::Guard read_lock(_read_lock);

	while(1) {
//		PDBG("%lu: looking up new postion. pt=%i, _item_count=%i, do_block=%i", Spartan::thread_get_id(), pt, _item_count, do_block);
		/* if the message we are looking for cpuld not be found */
		if(pt >= _item_count) {
			/* break if the queue request is non-blocking */
			if(!do_block) {
				PDBG("%lu: NON-blocking return", Spartan::thread_get_id());
				ret_call = Ipc_call();
				break;
			}

			PDBG("%lu: BLOCKING while pt=%i & _item_count=%i", Spartan::thread_get_id(), pt, _item_count);
			/**
			 * decrease _sem and therefor cause the thread to be blocked whenever
			 * 1) there is no message in the queue (on fresh entry of this function)
			 * 2) no unread message are in the queue (else)
			 * until it es woken up be a call to insert_new() by another thread
			 */
			_sem.down();
			PDBG("%lu: AWAKEN", Spartan::thread_get_id());
		}
		/* did we find the requested call? */
		if(cmp_fktn(_queue[pt], cmp_val)) {
//			PDBG("%lu: requested message found at position %lu", Spartan::thread_get_id(), pt);
			ret_call = _queue[pt];
			_remove_from_queue(pt);

			break;
		}

		/* if there is a message that we should take over governship of the Ipc_manager
		 *  remove the message from the queue and remember that should not block if
		 *  the message we are looking for is not in the queue */
		if(!_queue[pt].is_valid()) {
			_remove_from_queue(pt);
			do_block = false;
//			PDBG("%lu: being requested to take over governship of the ipc_manager", Spartan::thread_get_id());
		}
		else
			pt++;

//		PDBG("%lu: requested message not found at position %lu", Spartan::thread_get_id(), pt);
	}

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
//		_sem.down();
	}

	return call;
}


/* inserts a new call into the queue */
void
Ipc_call_queue::insert_new(Ipc_call new_call)
{
//	PDBG("%lu: starting to insert new call by thread %lu. Current _item_count=%lu, _sem.cnt()=%lu",
//	     new_call.dst_thread_id(), Spartan::thread_get_id(), _item_count, _sem.cnt());
	if(_item_count >= QUEUE_SIZE)
		throw Overflow();
	/* lock the queue for writing */
	Lock::Guard write_lock(_write_lock);

	_queue[_item_count++] = new_call;
	/* increase _sem to wake up the waiting thread */
	_sem.up();
	/* if no thread has been waiting redecrease _sem */
	if(_sem.cnt() > 0)
		_sem.down();
//	PDBG("%lu: new _item_count=%lu, _sem.cnt()=%lu", new_call.dst_thread_id(), _item_count, _sem.cnt());
}

