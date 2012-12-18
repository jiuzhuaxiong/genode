/*
 * \brief  Spartan specific methods
 * \author Tobias Börtitz
 * \date   2012-12-18
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__SPARTAN__METHODS_H_
#define _INCLUDE__SPARTAN__METHODS_H_

/* Spartan's ipc methods */
#include <abi/ipc/methods.h>

	enum {
		IPC_M_PHONE_HANDLE = IPC_FIRST_USER_METHOD + 0,
	};

#endif /* _INCLUDE__SPARTAN__METHODS_H_ */
