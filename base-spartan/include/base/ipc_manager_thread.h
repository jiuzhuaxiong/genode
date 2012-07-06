#ifndef _MANGER_THREAD_H_
#define _MANGER_THREAD_H_

#include <base/native_types.h>
#include <base/lock.h>

using namespace Genode;

enum {
	FULL_IPC_QUEUE = -20,
	TASK_KILLED = -21,
};

/* class to save incoming calls
 * since it is not recommended to use dynamical lists (malloc)
 * it has to be implemented in an array
 */
class Ipc_call_queue
{
	enum {
		QUEUE_SIZE = 32,
	};

	private:
		Ipc_call	_call_queue[QUEUE_SIZE];
		bool		_used[QUEUE_SIZE];
		int		_call_count;
		Lock		_queue_lock;

		Ipc_call	*_call_list[QUEUE_SIZE];

		int		_get_first_free_slot();
	public:
		explicit Ipc_call_queue();

		int		call_count(void) { return _call_count; }
		int		call_count(Native_thread_id rcv_thread_id);
		int		max_count(void) { return QUEUE_SIZE; }
		int		insert_new(Ipc_call new_call);
		Ipc_call	get_first(Native_thread_id rcv_thread_id);
		Ipc_call	get_last(void);
};

/* call implementing the manager thread which is exclusively looking into
 * the tasks answerbox, waiting for incoming calls and saving them.
 * Other threads obtain calls from this class
 * There is a maximum of one single Ipc_manager_thread per task
 */
class Ipc_manager_thread
{
	private:
		Native_thread_id	_thread_id;
		bool			_initialized;
		Ipc_call_queue		_call_queue;

		bool			_create();
		void			_wait_for_call();

	public:
		explicit Ipc_manager_thread()
		: _thread_id(Spartan::INVALID_ID), _initialized(false) {}
		~Ipc_manager_thread();

		/* Aks for number of stored calls for a specific thread */
		addr_t		get_call_count(Native_thread_id rcv_thread_id);

		/* non blocking call to receive 1 stored call for a specific
		 *  thread, if any exists
		 * returns true if an incoming call is stored in the supplied
		 *  structures, else returnes false*/
		Ipc_call	get_next_call(Native_thread_id rcv_thread_id);
		Ipc_call	wait_for_call(Native_thread_id rcv_thread_id);
};

#endif /* _MANGER_THREAD_H_ */
