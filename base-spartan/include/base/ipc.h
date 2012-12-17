 #ifndef _INCLUDE__BASE__IPC_H_
 #define _INCLUDE__BASE__IPC_H_

#include <base/ipc_generic.h>

#include <base/printf.h>

inline void Genode::Ipc_ostream::_marshal_capability(Genode::Native_capability const &cap)
{
	PDBG("Genode::Ipc_ostream::_marshal_capability NOT IMPLEMENTED");
}

inline void Genode::Ipc_istream::_unmarshal_capability(Genode::Native_capability &cap)
{
	PDBG("Genode::Ipc_istream::_unmarshal_capability NOT IMPLEMENTED");
}

 #endif /* _INCLUDE__BASE__IPC_H_ */
