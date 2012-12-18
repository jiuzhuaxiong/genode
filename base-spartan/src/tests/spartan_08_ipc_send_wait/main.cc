/*
 * \brief  Test for IPC send and wait via Genode's IPC framework
 * \author Norman Feske
 * \date   2009-03-26
 *
 * This program can be started as roottask replacement directly on the
 *  SPARTAN kernel.
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
#include <spartan/ipc.h>
#include <spartan/methods.h>
#include <spartan/errno.h>

/* local includes */
#include "../mini_env.h"

static Genode::Untyped_capability receiver_cap;


enum {
	PHONE_NAMESERV = 0,
	THREAD_NAMESERV = 5,
};


Genode::Native_task task_id_nameserv;
Genode::addr_t      phonehash_nameserv;


bool _register_with_nameserv(Genode::Thread_utcb* thread)
{
	Genode::addr_t msg_callid;
	Genode::Ipc_call call;

	msg_callid = Spartan::ipc_connect_to_me(PHONE_NAMESERV, THREAD_NAMESERV,
	                                                    Spartan::thread_get_id());
	call = thread->wait_for_reply();

	task_id_nameserv = call.snd_task_id();
	phonehash_nameserv = call.snd_phonehash();

	if ((IPC_GET_RETVAL(call.call()) == EOK) && (call.callid() & IPC_CALLID_ANSWERED)
			&& (call.callid() == (msg_callid | IPC_CALLID_ANSWERED)))
		return true;

	return false;
}

int _connect_to_myself(Genode::Thread_utcb* thread)
{
	if(!_register_with_nameserv(thread)) {
		PDBG("Could not register with nameserv!");
		return 0;
	}
	PDBG("Successfully registered with nameserv!");

	Genode::Ipc_call call;
	Genode::Native_thread_id mythread = Spartan::thread_get_id();

	Genode::addr_t msg_callid;
	msg_callid = Spartan::ipc_connect_me_to(PHONE_NAMESERV, mythread,
	                                        mythread);

	call = thread->wait_for_call(IPC_M_CONNECT_ME_TO);
	if(call.dest_thread_id() != mythread)
		Spartan::ipc_answer_0(call.callid(), call.snd_thread_id(),
		                      E__IPC_DESTINATION_UNKNOWN);

	Spartan::ipc_answer_1(call.callid(), call.snd_thread_id(), EOK,
	                      IPC_M_PHONE_HANDLE);
	call = thread->wait_for_reply();

	return call.cloned_phone();
}

/**
 * Sender thread, must not be started before 'receiver_cap' is initialized
 */
static void sender_thread_entry()
{
	Genode::Thread_utcb thread;
	thread.set_thread_id(Spartan::thread_get_id());
	Genode::Ipc_manager::singleton()->register_thread(&thread);

	static Genode::Msgbuf<256> sndbuf;
	static Genode::Ipc_ostream os(receiver_cap, &sndbuf);


	int a = 1, b = 2, c = 3;

	Genode::printf("sending a=%d, b=%d, c=%d\n", a, b, c);
	/* Since the for this test constructed Native_capability is a local,
	 * the capability is not send via ipc */
	os << a << b << c << /*os.dst() << */Genode::IPC_SEND;

	while(1);
}


/**
 * Main program
 */
int main()
{
	Genode::Thread_utcb thread;
	thread.set_thread_id(Spartan::thread_get_id());
	Genode::Ipc_manager::singleton()->register_thread(&thread);

	static Genode::Msgbuf<256> rcvbuf;
	static Genode::Ipc_istream is(&rcvbuf);

	/* make input stream capability known */
	receiver_cap = is;
	/* set id of capability so it can be marshalled */
	int myphone = _connect_to_myself(&thread);
	PDBG("acquired phone to myself is phone %i", myphone);
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
	is >> Genode::IPC_WAIT >> a >> b >> c/* >> cap*/;
	Genode::printf("received a=%d, b=%d, c=%d\n", a, b, c);

	while(1);
	Genode::printf("exiting main()\n");
	return 0;
}

