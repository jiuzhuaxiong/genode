/*
 * \brief  Test for IPC call via Genode's IPC framework
 * \author Norman Feske
 * \date   2009-03-26
 *
 * This program can be started as roottask replacement directly on the
 * OKL4 kernel. The main program plays the role of a server. It starts
 * a thread that acts as a client and performs an IPC call to the server.
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
#include <base/ipc_call.h>
#include <base/ipc_manager.h>

/* Spartan syscall includes */
#include <spartan/syscalls.h>

/* local includes */
#include "../mini_env.h"

using namespace Genode;

static Untyped_capability server_cap;

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
 * Client thread, must not be started before 'destination' is initialized
 */
static void client_thread_entry()
{
	Genode::Thread_utcb thread;
	Genode::Ipc_manager::singleton()->register_thread(&thread);

	Msgbuf<256> client_rcvbuf, client_sndbuf;
	Ipc_client client(server_cap, &client_sndbuf, &client_rcvbuf);

	printf("client sends call(11, 12, 13)\n");
	int res, d = 0, e = 0;
	res = (client << 11 << 12 << 13 << IPC_CALL >> d >> e).result();
	printf("client received reply d=%d, e=%d, res=%d\n", d, e, res);

	printf("client sends call(14, 15, 16)\n");
	res = (client << 14 << 15 << 16 << IPC_CALL >> d >> e).result();
	printf("client received reply d=%d, e=%d, res=%d\n", d, e, res);

	while(1);
}


/**
 * Main program
 */
int main()
{
	Genode::Thread_utcb thread;
	Genode::Ipc_manager::singleton()->register_thread(&thread);

	Msgbuf<256> server_rcvbuf, server_sndbuf;
	Ipc_server server(&server_sndbuf, &server_rcvbuf);

	/* make server capability known */
	server_cap = server;

	int myphone = _connect_to_myself();
	Genode::Ipc_destination dest = {Spartan::thread_get_id(), myphone};
	server_cap = Genode::Untyped_capability(dest, 2);

	/* create client thread, making a call to the server (us) */
	enum { THREAD_STACK_SIZE = 4096 };
	static int thread_stack[THREAD_STACK_SIZE];
	Spartan::thread_create((void*)&client_thread_entry, &thread_stack,
			THREAD_STACK_SIZE, "client_thread");

	/* infinite server loop */
	int a = 0, b = 0, c = 0;
	for (;;) {
		printf("server: reply_wait\n");

		server >> IPC_REPLY_WAIT >> a >> b >> c;
		printf("server: received a=%d, b=%d, c=%d, send reply %d, %d, res=33\n",
		       a, b, c, a + b + c, a*b*c);

		server << a + b + c << a*b*c;
		server.ret(33);
	}
	return 0;
}
