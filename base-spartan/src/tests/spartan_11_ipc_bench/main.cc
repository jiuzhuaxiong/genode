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
#include <base/ipc_message.h>
#include <base/ipc_manager.h>

/* Spartan syscall includes */
//#include <spartan/syscalls.h>
#include <spartan/errno.h>
#include <spartan/klog.h>

/* local includes */
#include "../mini_env.h"

using namespace Genode;


static Untyped_capability server_cap;

enum {
	PHONE_NAMESERV = 0,
	THREAD_NAMESERV = 5,
	LOOPS = 5000,
	RDTCS_CYCLES = 520,
	BUF_SIZE = 256,
};

inline uint64_t
rdtsc_amd64() {
	uint32_t lo, hi;
	__asm__ volatile (
		"xorl %%eax,%%eax\n\t"
		"cpuid\n\t"
		:
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	__asm__ volatile (
		"rdtsc" : "=a" (lo), "=d" (hi)
	);

	return (uint64_t)hi << 32 | lo;
}


Genode::Native_task task_id_nameserv;
Genode::addr_t      phonehash_nameserv;

uint64_t base1, base2, begin, end;

bool _register_with_nameserv(Genode::Thread_utcb* my_thread)
{
	Genode::addr_t msg_callid;
	Genode::Ipc_message msg;

	msg_callid = Spartan::ipc_connect_to_me(PHONE_NAMESERV, THREAD_NAMESERV, my_thread->thread_id());
//	                                                    Spartan::thread_get_id());
	msg = my_thread->msg_queue()->wait_for_answer(my_thread->thread_id());

	task_id_nameserv = msg.snd_task_id();
	phonehash_nameserv = msg.snd_phonehash();

	if ((IPC_GET_RETVAL(msg.call()) == EOK) && (msg.callid() & IPC_CALLID_ANSWERED)
			&& (msg.callid() == (msg_callid | IPC_CALLID_ANSWERED)))
		return true;

	return false;
}

int _connect_to_myself(Genode::Thread_utcb* my_thread)
{
	if(!_register_with_nameserv(my_thread)) {
		PDBG("Could not register with nameserv!");
		return 0;
	}
	PDBG("Successfully registered with nameserv!");

	Genode::Ipc_message msg;
	Genode::Native_thread_id mythread = my_thread->thread_id(); //Spartan::thread_get_id();

	Genode::addr_t msg_callid;
	msg_callid = Spartan::ipc_connect_me_to(PHONE_NAMESERV, mythread,
	                                        mythread);

	msg = my_thread->msg_queue()->wait_for_call(mythread, IPC_M_CONNECT_ME_TO);
	if(msg.dst_thread_id() != mythread)
		Spartan::ipc_answer_0(msg.callid(), msg.snd_thread_id(),
		                      E__IPC_DESTINATION_UNKNOWN);

	Spartan::ipc_answer_1(msg.callid(), msg.snd_thread_id(), EOK,
	                      IPC_M_PHONE_HANDLE);
	msg = my_thread->msg_queue()->wait_for_answer(mythread);

	return msg.cloned_phone();
}


/**
 * Client thread, must not be started before 'destination' is initialized
 */
static void client_thread_entry()
{
	Genode::Thread_utcb thread;
	uint64_t base, begin, end, average=0, max, max_idx, min, min_idx;
	uint64_t diff_array[LOOPS];
//	thread.set_thread_id(Spartan::thread_get_id());
//	Genode::Ipc_manager::singleton()->register_thread(&thread);

	Msgbuf<BUF_SIZE> client_rcvbuf, client_sndbuf;
	Ipc_client client(server_cap, &client_sndbuf, &client_rcvbuf);

	/* three warum up passes */
	for(int i=0; i<5; i++)
		rdtsc_amd64();

	for(int i=0; i<LOOPS; i++) {
//		printf("client sends call(11, 12, 13)\n");
		int res, d = 0, e = 0;
		begin = rdtsc_amd64();
		res = (client << 11 << 12 << 13 << IPC_CALL >> d >> e).result();
		end = rdtsc_amd64();
//		printf("client received reply d=%d, e=%d, res=%d\n", d, e, res);
		if(d!=14 || e!=15) {
			printf("WRONG VALUES RETURNED\n");
			Spartan::klog_printf("WRONG VALUES RETURNED\n");
		}
		diff_array[i] = end - begin - RDTCS_CYCLES;
		if(i==0) {
			min = diff_array[i];
			min_idx = i;
			max = diff_array[i];
			max_idx = i;
		} else {
			if(diff_array[i] < min) {
				min = diff_array[i];
				min_idx = i;
			}
			if(diff_array[i] > max) {
				max = diff_array[i];
				max_idx = i;
			}
		}
	}

	for(int i=0; i<LOOPS; i++) {
//		printf("diff %i = %llu\n", i, diff_array[i]);
		average += diff_array[i];
	}
	average = average / LOOPS;
	printf("average = %llu, min = %llu (%llu), max = %llu (%llu), while RDTCS_CYCLES = %llu\n", average, min, min_idx, max, max_idx, RDTCS_CYCLES);
	Spartan::klog_printf("average = %llu, min = %llu (%llu), max = %llu (%llu), while RDTCS_CYCLES = %llu\n", average, min, min_idx, max, max_idx, RDTCS_CYCLES);

	while(1);
}


/**
 * Main program
 */
int main()
{
	Genode::Thread_utcb thread;

	Msgbuf<BUF_SIZE> server_rcvbuf, server_sndbuf;
	Ipc_server server(&server_sndbuf, &server_rcvbuf);

	/* make server capability known */
	server_cap = server;

	int myphone = _connect_to_myself(&thread);
	Genode::Ipc_destination dest = {thread.thread_id(), myphone};
	server_cap = Genode::Untyped_capability(dest, 2);

	/* create client thread, making a call to the server (us) */
	enum { THREAD_STACK_SIZE = 4096 };
	static int thread_stack[THREAD_STACK_SIZE];
	Spartan::thread_create((void*)&client_thread_entry, &thread_stack,
			THREAD_STACK_SIZE, "client_thread");

	/* infinite server loop */
	int a = 0, b = 0, c = 0;
	for (;;) {
//		printf("server: reply_wait\n");

		server >> IPC_REPLY_WAIT >> a >> b >> c;
		server << 14 << 15;
		server.ret(33);

//		printf("server: received a=%d, b=%d, c=%d, sent reply %d, %d, res=33\n",
//		       a, b, c, a + b + c, a*b*c);
	}
	return 0;
}
