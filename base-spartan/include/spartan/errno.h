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


/* declare the offset where user defined errorcodes begin */
#define USER_ERROR_OFFSET         -256

/* error codes concerning the ipc framework */
#define E_IPC_CONNECTION_REJECTED   USER_ERROR_OFFSET - 0

#endif /* _INCLUDE__SPARTAN__ERRNO_H_ */
