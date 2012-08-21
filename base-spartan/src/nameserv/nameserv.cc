/*
 * \brief  Simple nameserver for usage with Spartan
 * \author Tobias Börtitz
 * \date   2012-08-14
 */

/*
 * Copyright (C) 2008-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/printf.h>
#include <base/native_types.h>
#include <base/ipc_call.h>

/* nameserv includes */
#include "mini_env.h"

/* spartan includes */
#include <spartan/syscalls.h>

using namespace Genode;


enum {
	TASK_ID_KERNEL = 1,
	TASK_ID_NAMESERV = 2,
	TASK_ID_CORE = 3,
};


/*****************
 * Nameserv main *
 *****************/

int main()
{
	Native_ipc_callid callid = 0;
	Native_ipc_call   native_call;
	Ipc_call          call;

	int               phone_core = -1;

	PDBG("--- nameserv started, accepting connections ---");

	while(1) {
		callid = Spartan::ipc_wait_for_call_timeout(&native_call, 0);
		call   = Ipc_call(callid, native_call);
		PDBG("new message received");

		if(call.snd_task_id() == TASK_ID_CORE
		   && call.call_method() == IPC_M_CONNECT_TO_ME) {
			phone_core = call.call_arg5();
			PDBG("[established connection to Core via phone %i]",
			     phone_core);
			Spartan::ipc_answer_0(call.callid(), 0);
		}
		else if (phone_core >= 0) {
			PDBG("Forwarding receivedcall to Core");
			/**
			 * TODO:
			 * optimize, so Spartan::ipc_forward_fast is used
			 * when possible (depends on the call's method?)
			 */
			Spartan::ipc_forward_slow(call.callid(), phone_core,
			                          call.call_method(),
			                          call.call_arg1(),
			                          call.call_arg2(),
			                          call.call_arg3(),
			                          call.call_arg4(),
			                          call.call_arg5(),
			                          IPC_FF_NONE);
		}
		else {
			/**
			 * this symbolizes the case, that there is no
			 * connection to Core. The call is answered to
			 * be failed
			 */
			Spartan::ipc_answer_0(call.callid(), -1);
		}
	}

	Spartan::exit(0);
}

