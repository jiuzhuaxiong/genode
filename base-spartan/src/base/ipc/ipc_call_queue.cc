#include <base/ipc_call_queue.h>
#include <base/printf.h>

using namespace Genode;

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
	printf("NOM _queue[pt].call_method()=%lu, imethod=%lu\n", _queue[pt].call_method(), imethod);

		/* check whether the current selected call is the one
		 * we are looking for */
		if((_queue[pt].call_method() == imethod) || (imethod == 0)) {
			ret_call = _queue[pt];
			break;
		}

		pt = (pt + 1) % QUEUE_SIZE;
		/* count how many times the semaphore has been decreased */
		sem_count++;
	}

	/* lock the complete queue while ordering the queue */
	Lock::Guard write_lock(_write_lock);
	printf("PT=%i\n", pt);
	for (int i=pt; i<_head; i = (i+1)%QUEUE_SIZE) {
		printf("i=%i, HEAD=%i, TAIL=%i\n", i, _head, _tail);
		if (pt != _tail) {
			printf("NOT TAIL=%i\n", _tail);
			_queue[i] = _queue[(i-1)%QUEUE_SIZE];
		}
	}
	/* decrease _head since there is one call less in the queue */
	_head = (_head-1)%QUEUE_SIZE;
	/* since we have taken 1 element from the queue we have to
	 * decrease the semaphore */
	_sem.down();

	/* decrease sem_count by 1 to avoid calling _unchecked_sem.down()
	 * before returning */
	sem_count--;
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
