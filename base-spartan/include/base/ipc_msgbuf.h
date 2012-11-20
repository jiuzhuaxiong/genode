#ifndef _INCLUDE__BASE__IPC_MSGBUF_H_
#define _INCLUDE__BASE__IPC_MSGBUF_H_

 namespace Genode {
	class Msgbuf_base { };

	template <unsigned BUF_SIZE>
		class Msgbuf : public Msgbuf_base { };
	}

 #endif /* _INCLUDE__BASE__IPC_MSGBUF_H_ */
