#include <base/thread.h>
#include <base/thread_utcb.h>

using namespace Genode;


Native_utcb *main_thread_utcb()
{
	static Native_utcb _main_utcb;
	return &_main_utcb;
}


Native_utcb *Thread_base::utcb()
{
	/*
	 * If 'utcb' is called on the object returned by 'myself',
	 * the 'this' pointer may be NULL (if the calling thread is
	 * the main thread). Therefore we allow this special case
	 * here.
	 */
	if (this == 0) return main_thread_utcb();

	return &_context->utcb;
}
