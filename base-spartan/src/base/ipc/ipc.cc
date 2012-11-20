#include <base/ipc.h>

using namespace Genode;

Ipc_ostream::Ipc_ostream(Native_capability dst, Msgbuf_base *snd_msg)
: Ipc_marshaller(0, 0) { }

void Ipc_istream::_wait() { }

Ipc_istream::Ipc_istream(Msgbuf_base *rcv_msg)
: Ipc_unmarshaller(0, 0) { }

Ipc_istream::~Ipc_istream() { }

void Ipc_client::_call() { }

Ipc_client::Ipc_client(Native_capability const &srv, Msgbuf_base *snd_msg,
                       Msgbuf_base *rcv_msg)
: Ipc_istream(rcv_msg), Ipc_ostream(srv, snd_msg), _result(0) { }

void Ipc_server::_wait() { }

void Ipc_server::_reply() { }

void Ipc_server::_reply_wait() { }

Ipc_server::Ipc_server(Msgbuf_base *snd_msg, Msgbuf_base *rcv_msg)
: Ipc_istream(rcv_msg), Ipc_ostream(Native_capability(), snd_msg) { }
