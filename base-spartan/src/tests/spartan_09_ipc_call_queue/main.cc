/*
 * \brief  Test for IPC send and wait via Genode's IPC framework
 * \author Norman Feske
 * \date   2009-03-26
 *
 * This program can be started as roottask replacement directly on the
 * OKL4 kernel.
 */

/*
 * Copyright (C) 2009-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/ipc.h>
#include <base/ipc_call_queue.h>
#include <base/ipc_manager.h>
#include <base/thread_utcb.h>

/* Spartan syscall includes */
#include <spartan/syscalls.h>

/* local includes */
#include "../mini_env.h"

enum {
	PHONE_NAMESERV = 0,
};

int my_phone;

bool _register_with_nameserv()
{
	Genode::Native_task	task_id_nameserv;
	Genode::addr_t		phonehash_nameserv;

	Spartan::ipc_connect_to_me(PHONE_NAMESERV, Spartan::thread_get_id(), 0,
			0, &task_id_nameserv, &phonehash_nameserv);
	return phonehash_nameserv ? true : false;
}

int _connect_to_myself()
{
	if(!_register_with_nameserv()) {
		Genode::printf("Could not register with nameserv!\n");
		return 0;
	}

	int              ret;
	Genode::Ipc_call call;
	Genode::Native_thread_id mythread = Spartan::thread_get_id();
	ret = Spartan::ipc_call_sync_2_0(PHONE_NAMESERV, 
		/*IPC_CONNECTION_REQUEST*/ 30, mythread, mythread);

	/* connection endpoint is not known */
	if(ret<0)
		return ret;

	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call(IPC_M_CONNECTION_CLONE);
	ret = call.cloned_phone();
	Spartan::ipc_answer_0(call.callid(), 0);

	/* return the aquired phone */
	return ret;
}

/**
 * Sender thread
 */
static void sender_thread_entry()
{
	Genode::Thread_utcb thread;
	Genode::Ipc_manager::singleton()->register_thread(&thread);

	/*
	 * 1. Test: after 3 sekonds of sleeping
	 * send a call with rancom number to receiving thread
	 */
	Genode::printf("\n============ Test 1 ============\n\n");
	for(int i=1; i<4; i++) {
		Genode::printf("%i ...", i);
		Spartan::usleep(1000000);
	}
	Genode::printf("\n");
	Spartan::ipc_call_sync_2_0(my_phone, 20, Spartan::thread_get_id(), 6);

	/*
	 * 2. Test: a) send not waitet for method
	 *          b) wait 3 sekonds
	 *          c) send mehod waitet for
	 */
	Genode::printf("\n============ Test 2 ============\n\n");
	Spartan::ipc_call_async_2(my_phone, 20, Spartan::thread_get_id(), 6);
	for(int i=1; i<4; i++) {
		Genode::printf("%i ...", i);
		Spartan::usleep(1000000);
	}
	Genode::printf("\n");
	Spartan::ipc_call_sync_2_0(my_phone, 30, Spartan::thread_get_id(), 6);

	/*
	 * 3. Test: a) send not waitet for method
	 *          b) send method waitet for second
	 *          c) send method waitet for first
	 */
	Genode::printf("\n============ Test 3 ============\n\n");
	Spartan::ipc_call_async_2(my_phone, 40, Spartan::thread_get_id(), 6);
	Spartan::ipc_call_async_2(my_phone, 30, Spartan::thread_get_id(), 6);
	Spartan::ipc_call_async_2(my_phone, 20, Spartan::thread_get_id(), 6);

	while(1);
}


/**
 * Main program
 */
int main()
{
	/* define own thread_utcb and register with Ipc_manager
	 * so we can receive calls */
	Genode::Thread_utcb thread;
	Genode::Ipc_manager::singleton()->register_thread(&thread);

	Genode::Ipc_call call;

	/* create connection to myself so calls can be send */
	my_phone = _connect_to_myself();

	/* create sender thread, sending to destination (us) */
	enum { THREAD_STACK_SIZE = 4096 };
	static int thread_stack[THREAD_STACK_SIZE];
	Spartan::thread_create((void*)&sender_thread_entry, &thread_stack,
		THREAD_STACK_SIZE, "sender_thread");

	/*
	 * 1. Test: simply wait for an incomming call
	 *  message is send after 1 second of sleep from the sender thread
	 */
	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call();
	Genode::printf("ReceiverThread:\t got call with method %lu\n", call.call_method());
	Spartan::ipc_answer_0(call.callid(), 0);

	/*
	 * 2. Test: a) wait for a specific call with method 30
	 *             (just a numer without any meaning - for testing issues)
	 *          b) wait for any incomming call
	 */
	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call(30);
	Genode::printf("ReceiverThread:\t got first call with method %lu\n", call.call_method());
	Spartan::ipc_answer_0(call.callid(), 0);
	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call();
	Genode::printf("ReceiverThread:\t got second call with method %lu\n", call.call_method());
	Spartan::ipc_answer_0(call.callid(), 0);

	/*
	 * 3. Test: a) sleep for 5 sekonds
	 *          b) wait for a specific call with method 20
	 *          c) wait for a specific call with method 30
	 *          d) wait any incomming call (40)
	 */
	for(int i=1; i<6; i++) {
		Genode::printf("%i ...", i);
		Spartan::usleep(1000000);
	}
	Genode::printf("\n");
	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call(20);
	Genode::printf("ReceiverThread:\t got first call with method %lu\n", call.call_method());
	Spartan::ipc_answer_0(call.callid(), 0);
	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call(30);
	Genode::printf("ReceiverThread:\t got second call with method %lu\n", call.call_method());
	Spartan::ipc_answer_0(call.callid(), 0);
	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call();
	Genode::printf("ReceiverThread:\t got third call with method %lu\n", call.call_method());
	Spartan::ipc_answer_0(call.callid(), 0);


	while(1);
	Genode::printf("exiting main()\n");
	return 0;
}

