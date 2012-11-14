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
#include <base/semaphore.h>
<<<<<<< HEAD
#include <base/native_types.h>
=======
#include <base/native_types>
>>>>>>> cae4f6b3f45054d03b1202798d7ae34fbcfc4055

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

Genode::Semaphore _sem;

/**
 * Sender thread
 */
static void sender_thread_entry()
{
	Genode::Thread_utcb thread;
	Genode::Ipc_manager::singleton()->register_thread(&thread);

	/*
	 * 1. Test: after 2 sekonds of sleeping
	 * send a call with rancom number to receiving thread
	 */
	Genode::printf("\n============ Test 1 ============\n\n");
	for(int i=1; i<3; i++) {
		Genode::printf("%i ...", i);
		Spartan::usleep(1000000);
	}
	Genode::printf("\n");
	Spartan::ipc_call_sync_2_0(my_phone, 20, thread.thread_id(), 6);
	_sem.down();

	/*
	 * 2. Test: send 2 messages with an intaval of 1 second
	 */
	Genode::printf("\n============ Test 2 ============\n\n");
	Spartan::ipc_call_sync_2_0(my_phone, 20, thread.thread_id(), 6);
	Spartan::usleep(1000000);
	Spartan::ipc_call_sync_2_0(my_phone, 20, thread.thread_id(), 6);
	_sem.down();

	/*
	 * 3. Test: a) send not waitet for method
	 *          b) wait 2 sekonds
	 *          c) send mehod waitet for
	 */
	Genode::printf("\n============ Test 3 ============\n\n");
	Spartan::ipc_call_async_2(my_phone, 20, thread.thread_id(), 6);
	for(int i=1; i<3; i++) {
		Genode::printf("%i ...", i);
		Spartan::usleep(1000000);
	}
	Genode::printf("\n");
	Spartan::ipc_call_sync_2_0(my_phone, 30, thread.thread_id(), 6);
	_sem.down();

	/*
	 * 4. Test: a) send not waitet for method
	 *          b) send method waitet for second
	 *          c) send method waitet for first
	 */
	Genode::printf("\n============ Test 4 ============\n\n");
	Spartan::ipc_call_async_2(my_phone, 40, thread.thread_id(), 6);
	Spartan::ipc_call_async_2(my_phone, 30, thread.thread_id(), 6);
	Spartan::ipc_call_async_2(my_phone, 20, thread.thread_id(), 6);
	_sem.down();

	/*
	 * 5. Test: send 3 messages, the middle one is first being waited for
	 */
	Genode::printf("\n============ Test 5 ============\n\n");
	Spartan::ipc_call_async_2(my_phone, 40, thread.thread_id(), 6);
	Spartan::ipc_call_async_2(my_phone, 20, thread.thread_id(), 6);
	Spartan::ipc_call_async_2(my_phone, 30, thread.thread_id(), 6);
	_sem.down();

	/*
	 * 6. Test: send 3 messages, the middle one is first being waited for
	 */
	Genode::printf("\n============ Test 6 ============\n\n");
	Spartan::ipc_call_async_2(my_phone, 30, thread.thread_id(), 6);
	Spartan::ipc_call_async_2(my_phone, 20, thread.thread_id(), 6);
	Spartan::ipc_call_async_2(my_phone, 40, thread.thread_id(), 6);
	_sem.down();

	/*
	 * 7. Test: send more messages, forcing the ringbuffer to switch from
	 *  'high' queue places to 'low' ones
	 */
	Genode::printf("\n============ Test 7 ============\n\n");
	for(int i=21; i<26; i++)
		Spartan::ipc_call_sync_2_0(my_phone, i, thread.thread_id(), 6);
	Spartan::ipc_call_async_2(my_phone, 29, thread.thread_id(), 6);
	Spartan::ipc_call_async_2(my_phone, 28, thread.thread_id(), 6);
	Spartan::ipc_call_async_2(my_phone, 27, thread.thread_id(), 6);
	Spartan::ipc_call_async_2(my_phone, 30, thread.thread_id(), 6);
	_sem.down();

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
	_sem.up();

	/*
	 * 2. Test: simply wait for 2 random incomming messages
	 */
	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call();
	Genode::printf("ReceiverThread:\t got first call with method %lu\n", call.call_method());
	Spartan::ipc_answer_0(call.callid(), 0);
	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call();
	Genode::printf("ReceiverThread:\t got second call with method %lu\n", call.call_method());
	Spartan::ipc_answer_0(call.callid(), 0);
	_sem.up();

	/*
	 * 3. Test: a) wait for a specific call with method 30
	 *             (just a numer without any meaning - for testing issues)
	 *          b) wait for any incomming call
	 */
	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call(30);
	Genode::printf("ReceiverThread:\t got first call with method %lu\n", call.call_method());
	Spartan::ipc_answer_0(call.callid(), 0);
	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call();
	Genode::printf("ReceiverThread:\t got second call with method %lu\n", call.call_method());
	Spartan::ipc_answer_0(call.callid(), 0);
	_sem.up();

	/*
	 * 4. Test: a) sleep for 2 sekonds
	 *          b) wait for a specific call with method 20
	 *          c) wait for a specific call with method 30
	 *          d) wait any incomming call (40)
	 */
	for(int i=1; i<3; i++) {
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
	_sem.up();

	/*
	 * 5. Test: wait for a seconds to receive 3 calls
	 *          a) retrive for the second one
	 *          b) retrieve the first one
	 *          c) retrieve the last one
	 */
	Spartan::usleep(1000000);
	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call(20);
	Genode::printf("ReceiverThread:\t got first call with method %lu\n", call.call_method());
	Spartan::ipc_answer_0(call.callid(), 0);
	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call(30);
	Genode::printf("ReceiverThread:\t got second call with method %lu\n", call.call_method());
	Spartan::ipc_answer_0(call.callid(), 0);
	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call();
	Genode::printf("ReceiverThread:\t got third call with method %lu\n", call.call_method());
	Spartan::ipc_answer_0(call.callid(), 0);
	_sem.up();

	/*
	 * 6. Test: wait for a seconds to receive 3 calls
	 *          a) retrive for the second one
	 *          b) retrieve the last one
	 *          c) retrieve the fist one
	 */
	Spartan::usleep(1000000);
	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call(20);
	Genode::printf("ReceiverThread:\t got first call with method %lu\n", call.call_method());
	Spartan::ipc_answer_0(call.callid(), 0);
	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call(30);
	Genode::printf("ReceiverThread:\t got second call with method %lu\n", call.call_method());
	Spartan::ipc_answer_0(call.callid(), 0);
	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call();
	Genode::printf("ReceiverThread:\t got third call with method %lu\n", call.call_method());
	Spartan::ipc_answer_0(call.callid(), 0);
	_sem.up();

	/*
	 * 7. Test: retrieve mor messages from first to last
	 * in the end test ordering of the queue when switching from high to low queue places
	 */
	for(int i=21; i<26; i++)
	{
		call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call(i);
		Genode::printf("ReceiverThread:\t got call with method %lu\n", call.call_method());
		Spartan::ipc_answer_0(call.callid(), 0);
	}
	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call(27);
	Genode::printf("ReceiverThread:\t got first call with method %lu\n", call.call_method());
	Spartan::ipc_answer_0(call.callid(), 0);
	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call(28);
	Genode::printf("ReceiverThread:\t got first call with method %lu\n", call.call_method());
	Spartan::ipc_answer_0(call.callid(), 0);
	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call(29);
	Genode::printf("ReceiverThread:\t got first call with method %lu\n", call.call_method());
	Spartan::ipc_answer_0(call.callid(), 0);
	call = Genode::Ipc_manager::singleton()->my_thread()->wait_for_call(30);
	Genode::printf("ReceiverThread:\t got first call with method %lu\n", call.call_method());
	Spartan::ipc_answer_0(call.callid(), 0);
	_sem.up();


	while(1);
	Genode::printf("exiting main()\n");
	return 0;
}

