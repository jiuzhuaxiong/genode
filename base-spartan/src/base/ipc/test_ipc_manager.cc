#include <base/ipc_manager.h>
#include <base/thread.h>

using namespace Genode;


template<int QUEUE_SIZE>
Thread_utcb* Thread_buffer<QUEUE_SIZE>::exists_global_threadid(Native_thread_id thread_id)
{
	Lock::Guard lock(_thread_lock);

	for(int i=0; i<_thread_count; i++)
		if(_threads[i]->global_thread_id() == thread_id)
			return _threads[i];
	return 0;
}


Native_utcb*
Ipc_manager::my_utcb()
{
	Native_utcb* utcb = _threads.exists_global_threadid(Spartan::thread_get_id());
	if(utcb == 0)
		return Thread_base::myself()->utcb();

	return utcb;
}

