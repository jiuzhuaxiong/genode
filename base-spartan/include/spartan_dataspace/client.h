/*
 * \brief  Linux-specific dataspace client interface
 * \author Norman Feske
 * \date   2006-05-11
 */

/*
 * Copyright (C) 2006-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__LINUX_DATASPACE__CLIENT_H_
#define _INCLUDE__LINUX_DATASPACE__CLIENT_H_

#include <dataspace/client.h>
#include <spartan_dataspace/spartan_dataspace.h>
#include <base/rpc_client.h>
#include <util/string.h>

namespace Genode {

	struct Spartan_dataspace_client : Rpc_client<Linux_dataspace>
	{
		explicit Spartan_dataspace_client(Dataspace_capability ds)
		: Rpc_client<Sparatan_dataspace>(static_cap_cast<Spartan_dataspace>(ds)) { }


		/*********************************
		 ** Generic dataspace interface **
		 *********************************/

		size_t size()      { return call<Rpc_size>();      }
		addr_t phys_addr() { return call<Rpc_phys_addr>(); }
		bool   writable()  { return call<Rpc_writable>();  }


		/****************************************
		 ** Linux-specific dataspace interface **
		 ****************************************/

		Filename fname() { return call<Rpc_fname>(); }
	};
}

#endif /* _INCLUDE__LINUX_DATASPACE__CLIENT_H_ */
