/*
 * \brief  Spartan-specific platform thread implementation
 * \author Norman Feske
 * \date   2007-10-15
 */

/*
 * Copyright (C) 2007-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <util/token.h>
#include <util/misc_math.h>
#include <base/printf.h>

/* local includes */
#include "platform_thread.h"

using namespace Genode;


typedef Token<Scanner_policy_identifier_with_underline> Tid_token;


Platform_thread::Platform_thread(const char *name, unsigned, addr_t)
{
	PWRN("%s: Not implemented", __PRETTY_FUNCTION__);
}


void Platform_thread::cancel_blocking()
{
	PWRN("%s: Not implemented", __PRETTY_FUNCTION__);
//	PDBG("send cancel-blocking signal to %ld\n", _tid);
//	lx_tgkill(_pid, _tid, LX_SIGUSR1);
}


void Platform_thread::pause()
{
	PWRN("%s: Not implemented", __PRETTY_FUNCTION__);
}


void Platform_thread::resume()
{
	PWRN("%s: Not implemented", __PRETTY_FUNCTION__);
}
