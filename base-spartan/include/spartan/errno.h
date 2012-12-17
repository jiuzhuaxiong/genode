/*
 * \brief  Spartan futex specific syscalls
 * \author Tobias Börtitz
 * \date   2012-11-29
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__SPARTAN__ERRNO_H_
#define _INCLUDE__SPARTAN__ERRNO_H_

/* SPARTAN's error codes */
#include <abi/errno.h>

enum {
	/* declare the offset where user defined errorcodes begin */
	USERSPACE_ERROR_OFFSET           = -256,

/* error codes concerning the ipc framework */
	E__IPC_CONNECTION_REJECTED  = USERSPACE_ERROR_OFFSET - 0,
	E__IPC_DESTINATION_UNKNOWN  = USERSPACE_ERROR_OFFSET - 1,
	E__IPC_CALL_QUEUE_FULL      = USERSPACE_ERROR_OFFSET - 2,


	E__THREAD_KILLED            = USERSPACE_ERROR_OFFSET - 3,
};

#endif /* _INCLUDE__SPARTAN__ERRNO_H_ */
