#include <base/thread.h>
#include <base/thread_utcb.h>
#include <base/ipc_manager.h>

#include <spartan/syscalls.h>

using namespace Genode;


Native_utcb *main_thread_utcb()
{
	static Native_utcb _main_utcb;
	return &_main_utcb;
}

void Thread_base::_init_platform_thread()
{
	_tid = Spartan::thread_get_id();
	Ipc_manager::singleton()->register_thread();
}

void Thread_base::_deinit_platform_thread()
{
	Ipc_manager::singleton()->unregister_thread();
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
