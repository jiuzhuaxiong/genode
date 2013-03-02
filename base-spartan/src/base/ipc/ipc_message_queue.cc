#include <base/printf.h>
#include <base/thread.h>

#include <base/ipc_message_queue.h>
#include <base/ipc_manager.h>

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
_cmp_imethod(Ipc_message msg, addr_t imethod)
{
	return (!msg.is_answer()
	        && (msg.method() == imethod
	         || imethod == 0));
}


/* check whether a call is an answer to the specified callid */
bool
_cmp_answer_callid(Ipc_message msg, Native_ipc_callid callid)
{
//	PDBG("is %lu answer to %lu?: %i", msg.callid(), callid, msg.is_answer_to(callid));
	return (msg.is_answer_to(callid)
	        || callid == 0);
}

/*********************
 * private functions *
 *********************/

/**
 * remove the element at the given position from the queue.
 * make shure to safe the element in question before calling this function!
 */
void
Ipc_message_queue::_remove_from_queue(addr_t pos)
{
	/* lock queue for writing */
	Lock::Guard write_lock(_write_lock);
	/* reorder queue removing the element in question */
	PDBG("%lu: removing element %lu with callid %lu from queue", Spartan::thread_get_id(), pos, _queue[pos].callid());
	for(; pos<(_item_count-1); pos++)
		_queue[pos] = _queue[pos+1];
	/* reduce the _item_count to its new value */
	_item_count--;
}

/**
 * get first occurence of a specific call from th queue
 *  comparison is defined through a function pointer
 */
Ipc_message
Ipc_message_queue::_get_first(Native_thread_id thread_id, addr_t cmp_val,
                              bool (*cmp_fktn)(Ipc_message, addr_t))
{
	Ipc_message ret_msg;
	addr_t   pt = 0;

	/* lock queue for reading */
	Lock::Guard read_lock(_read_lock);

	while(1) {
//		PDBG("%lu: looking up new postion. pt=%i, _item_count=%i, do_block=%i", Spartan::thread_get_id(), pt, _item_count, do_block);
		/* if the message we are looking for cpuld not be found */
		if(pt >= _item_count) {
			PDBG("%lu: trying to obtain governorship of Ipc_manager", Spartan::thread_get_id());
			Ipc_manager::singleton()->get_call(thread_id);
			if(pt >= _item_count)
				PDBG("%lu: BLOCKING", Spartan::thread_get_id());
		}

		/**
		 * decrease _sem and therefor cause the thread to be blocked whenever
		 * 1) there is no message in the queue (on fresh entry of this function)
		 * 2) no unread message are in the queue (else)
		 * until it es woken up be a call to insert_new() by another thread
		 */
		_sem.down();

		if(pt >= _item_count)
			PDBG("%lu: AWAKEN", Spartan::thread_get_id());

		/* did we find the requested call? */
		if(cmp_fktn(_queue[pt], cmp_val)) {
//			PDBG("%lu: requested message found at position %lu", Spartan::thread_get_id(), pt);
			ret_msg = _queue[pt];
			_remove_from_queue(pt);

			break;
		}

		/* if there is a message that we should take over governship of the Ipc_manager
		 *  remove the message from the queue and remember that should not block if
		 *  the message we are looking for is not in the queue */
		if(!_queue[pt].is_valid()) {
			_remove_from_queue(pt);
		}
		else
			pt++;

//		PDBG("%lu: requested message not found at position %lu", Spartan::thread_get_id(), pt);
	}

	/* prepare the semaphore for next search by re-increaseing
	 *  the semaphore so it consists of the exact same
	 *  count as _sem */
	for(int i=0; i<pt; i++)
		_sem.up();

	return ret_msg;
}

/********************
 * public functions *
 ********************/

/* get the first call with the specified imethod */
Ipc_message
Ipc_message_queue::wait_for_call(Native_thread_id thread_id,
                                 addr_t imethod)
{
	return _get_first(thread_id, imethod, &_cmp_imethod);
}


/* get the first reply to the sepcified callid */
Ipc_message
Ipc_message_queue::wait_for_answer(Native_thread_id thread_id,
                                   Native_ipc_callid callid)
{
	return _get_first(thread_id, callid, &_cmp_answer_callid);
}


/* get the last call/answer without any checking */
Ipc_message
Ipc_message_queue::get_last(void)
{
	Ipc_message call;
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
Ipc_message_queue::insert(Ipc_message new_call)
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
//	PDBG("%lu: new _item_count=%lu, _sem.cnt()=%lu", new_call.dst_thread_id(), _item_count, _sem.cnt());
}

bool
Ipc_message_queue::is_waiting()
{
//	PDBG("_sem.cnt() = %i", _sem.cnt());
	return (_sem.cnt() < 0);
}

