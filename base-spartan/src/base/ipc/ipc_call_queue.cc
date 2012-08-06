#include <base/ipc_call_queue.h>
#include <base/printf.h>

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

void
Ipc_call_queue::insert_new(Ipc_call new_call)
{
	printf("Ipc_call_queue:\tstarting to insert new call\n");
	/* lock the queue for writing */
	Lock::Guard write_lock(_write_lock);
	try {
		add(new_call);
		_unchecked_sem.up();
	} catch (Overflow) {
		throw Overflow();
	}
}

Ipc_call
Ipc_call_queue::get_first(addr_t imethod)
{
	Ipc_call ret_call;
	int      pt = _tail;
	int      sem_count = 0;

	/* lock the queue for reading
	 * writing is allowed*/
	Lock::Guard read_lock(_read_lock);
	while(1) {
		/* locks whenever the queue is empty or the call we 
		 *  are waiting for is not in the queue (pt = _head+1)
		 * unlocks when a new element has been inserted 
		 *  thus aquiring the _write_lock is not needed */
		_unchecked_sem.down();
	printf("NOM _queue[%i].call_method()=%lu, imethod=%lu\n", pt, _queue[pt].call_method(), imethod);

		/* check whether the current selected call is the one
		 * we are looking for */
		if((_queue[pt].call_method() == imethod) || (imethod == 0)) {
			break;
		}

		pt = (pt + 1) % QUEUE_SIZE;
		/* count how many times the semaphore has been decreased */
		sem_count++;
	}

	Lock::Guard write_lock(_write_lock);
	printf("PT=%i, HEAD=%i, TAIL=%i\n", pt, _head, _tail);
	if(pt == _tail)
		ret_call = get();
	else {
		ret_call = _queue[pt];
		for (int i=pt; _i_lt_head(i, _head, _tail); i = (i+1)%QUEUE_SIZE) {
			printf("i=%i, HEAD=%i, TAIL=%i\n", i, _head, _tail);
			_queue[i] = _queue[(i+1)%QUEUE_SIZE];
		}
		/* decrease _head since there is one call less in the queue */
		_head = (_head-1)%QUEUE_SIZE;
		/* since we have taken 1 element from the queue we have to
		 * decrease the semaphore */
		_sem.down();
	}

	/* prepare the semaphore for next search by re-increaseing
	 *  the semaphore so it consists of the exact same
	 *  count as _sem */
	for(int i=0; i<sem_count; i++)
		_unchecked_sem.up();

	return ret_call;
}

Ipc_call
Ipc_call_queue::get_last(void)
{
	/* lock the queue for reading
	 * writing is still allowed */
	Lock::Guard read_lock(_read_lock);

	_unchecked_sem.down();

	return get();
}
