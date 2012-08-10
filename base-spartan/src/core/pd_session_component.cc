/**
 * \brief  Core implementation of the PD session interface
 * \author Christian Helmuth
 * \date   2006-07-17
 *
 * FIXME arg_string and quota missing
 */

/*
 * Copyright (C) 2006-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode */
#include <base/printf.h>

/* Core */
#include <pd_session_component.h>

using namespace Genode;


Pd_session_component::Pd_session_component(Rpc_entrypoint *thread_ep,
                                           const char *args)
{
	PERR("%s: Not implemented", __PRETTY_FUNCTION__);
	_pid = Arg_string::find_arg(args, "PID").long_value(0);
}


Pd_session_component::~Pd_session_component()
{
	PERR("%s: Not implemented", __PRETTY_FUNCTION__);
//	if (_pid)
//		lx_kill(_pid, 9);
}


int Pd_session_component::bind_thread(Thread_capability)
{
	PERR("%s: Not implemented", __PRETTY_FUNCTION__);
	return -1;
}


int Pd_session_component::assign_parent(Parent_capability)
{
	PERR("%s: Not implemented", __PRETTY_FUNCTION__);
	return -1;
}
