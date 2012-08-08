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
#include <base/ipc_msgbuf.h>
#include <base/ipc_manager.h>
#include <base/native_types.h>

/* Spartan syscall includes */
#include <spartan/syscalls.h>

/* local includes */
#include "../mini_env.h"

static Genode::Untyped_capability receiver_cap;


enum {
	PHONE_NAMESERV = 0,
};

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
 * Sender thread, must not be started before 'receiver_cap' is initialized
 */
static void sender_thread_entry()
{
	Genode::Thread_utcb thread;
	Genode::Ipc_manager::singleton()->register_thread(&thread);
	static Genode::Msgbuf<256> sndbuf;
	static Genode::Ipc_ostream os(receiver_cap, &sndbuf);


	int a = 1, b = 2, c = 3;

	Genode::printf("sending a=%d, b=%d, c=%d\n", a, b, c);
	/* Since the for this test constructed Native_capability is a local,
	 * the capability is not send via ipc */
	os << a << b << c << os.dst() << Genode::IPC_SEND;

	while(1);
}


/**
 * Main program
 */
int main()
{
	Genode::Thread_utcb thread;
	static Genode::Msgbuf<256> rcvbuf;
	static Genode::Ipc_istream is(&rcvbuf);

	Genode::Ipc_manager::singleton()->register_thread(&thread);

	/* make input stream capability known */
	receiver_cap = is;
	/* set id of capability so it can be marshalled */
	int myphone = _connect_to_myself();
	Genode::Ipc_destination dest = {Spartan::thread_get_id(), myphone};
	receiver_cap = Genode::Untyped_capability(dest, 2);

	/* create sender thread, sending to destination (us) */
	enum { THREAD_STACK_SIZE = 4096 };
	static int thread_stack[THREAD_STACK_SIZE];
	Spartan::thread_create((void*)&sender_thread_entry, &thread_stack,
		THREAD_STACK_SIZE, "sender_thread");

	/* wait for incoming IPC */
	int a = 0, b = 0, c = 0;
	Genode::Native_capability cap;
	is >> Genode::IPC_WAIT >> a >> b >> c >> cap;
	Genode::printf("received a=%d, b=%d, c=%d\n", a, b, c);

	while(1);
	Genode::printf("exiting main()\n");
	return 0;
}

