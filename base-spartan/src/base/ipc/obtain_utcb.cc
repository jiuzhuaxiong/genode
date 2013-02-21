/*
 * \brief  IPC implementation for Spartan
 * \author Norman Feske
 * \author Tobias Börtitz
 * \date   2009-03-25
 */

/*
 * Copyright (C) 2009-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <base/thread.h>

using namespace Genode;

Thread_utcb* _obtain_utcb()
{
	return Thread_base::myself()->utcb();
}
