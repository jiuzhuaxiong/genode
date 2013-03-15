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
	return (msg.is_valid()
	        && !msg.is_answer()
	        && (msg.method() == imethod
	         || imethod == 0));
}


/* check whether a call is an answer to the specified callid */
bool
_cmp_answer_callid(Ipc_message msg, Native_ipc_callid callid)
{
//	PDBG("is %lu answer to %lu?: %i", msg.callid(), callid, msg.is_answer_to(callid));
	return (msg.is_valid()
	        && msg.is_answer_to(callid)
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
Ipc_message_queue::_get_first(Native_thread_id thread_id)
{
	Ipc_message ret_msg;

	addr_t pt = 0;
	_msg_pt = 0;

	/* lock queue for reading */
	Lock::Guard read_lock(_read_lock);

	while(1) {
//		PDBG("%lu: looking up new postion. pt=%i, _item_count=%i, do_block=%i", Spartan::thread_get_id(), pt, _item_count, do_block);
		/**
		 * if the message we are looking for could not be found
		 *  try to obtain the governorship over the Ipc_manager
		 * if the governorship could not be obtained, block the 
		 *   thread to actively wait for incoming messages
		 */
		if(pt >= _item_count) {
			PDBG("%lu: trying to obtain governorship of Ipc_manager", Spartan::thread_get_id());
			if(!Ipc_manager::singleton()->get_call(thread_id)) {
				PDBG("%lu: BLOCKING", Spartan::thread_get_id());
				_sem.down();
			}

			/**
			 * if the desired message has been inserted while being the governor
			 *  of the IPC manager or the thread has been blocked, the desired 
			 *  message will be located at the position _msg_pt is pointing to
			 */
			if(_msg_pt > pt)
				pt = _msg_pt;
		}

		/* do we find the requested message a the current postion? */
		if(_cmp_fktn(_queue[pt], _cmp_val)) {
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
	}

	/* reset the semaphore for the next search */
	while(_sem.cnt() > 0)
		_sem.down();

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
	Ipc_message msg;

	_cmp_val = imethod;
	_cmp_fktn = &_cmp_imethod;
	msg = _get_first(thread_id);
	_cmp_fktn = 0;
	_cmp_val = 0;

	return msg;
}


/* get the first reply to the sepcified callid */
Ipc_message
Ipc_message_queue::wait_for_answer(Native_thread_id thread_id,
                                   Native_ipc_callid callid)
{
	Ipc_message msg;

	_cmp_val = callid;
	_cmp_fktn = &_cmp_answer_callid;
	msg = _get_first(thread_id);
	_cmp_fktn = 0;
	_cmp_val = 0;

	return msg;
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
bool
Ipc_message_queue::insert(Ipc_message new_call)
{
//	PDBG("%lu: starting to insert new call by thread %lu. Current _item_count=%lu, _sem.cnt()=%lu",
//	     new_call.dst_thread_id(), Spartan::thread_get_id(), _item_count, _sem.cnt());
	if(_item_count >= QUEUE_SIZE)
		throw Overflow();

	/* lock the queue for writing */
	Lock::Guard write_lock(_write_lock);

	_queue[_item_count++] = new_call;

	if(_cmp_fktn
	   && _cmp_fktn(new_call, _cmp_val)) {
		/* set the message pointer to the desired message
		 *  so it will be found instantly */
		_msg_pt = _item_count - 1;
		/* signale that the desired message has been inserted */
		_sem.up();
		return true;
	}

	return false;
}

bool
Ipc_message_queue::is_waiting()
{
//	PDBG("_sem.cnt() = %i", _sem.cnt());
	return (_sem.cnt() < 0);
}

