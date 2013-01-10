 #ifndef _INCLUDE__BASE__IPC_H_
 #define _INCLUDE__BASE__IPC_H_

#include <base/ipc_generic.h>

inline void Genode::Ipc_ostream::_marshal_capability(Genode::Native_capability const &cap)
{
	bool local = cap.local();
	long id    = local ? (long)cap.local() : cap.local_name();

	_write_to_buf(local);
	_write_to_buf(id);

	/* only transfer kernel-capability if it's no local capability and valid */
//	if (!local && id) {
		_snd_msg->cap_append(cap);
//	}
}

inline void Genode::Ipc_istream::_unmarshal_capability(Genode::Native_capability &cap)
{
	bool local = false;
	long id    = 0;

	/* extract capability id from message buffer and whether it's a local cap */
	_read_from_buf(local);
	_read_from_buf(id);

	/* if it's a local capability, the pointer is marshalled in the id */
//	if (local) {
//		cap = Capability<Native_capability>::local_cap((Native_capability*)id);
//		return;
//	}

	/* if id is zero, an invalid capability was tranfered */
	if (!id) {
		cap = Native_capability();
		return;
	}

	_rcv_msg->cap_get_by_id(&cap, id);
}

 #endif /* _INCLUDE__BASE__IPC_H_ */
