/*
 * \brief  Implementation of the Thread API
 * \author Norman Feske
 * \date   2010-01-11
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <base/thread.h>
#include <base/env.h>
#include <base/snprintf.h>
#include <util/string.h>
#include <util/misc_math.h>

namespace Fiasco {
#include <l4/sys/utcb.h>
}

using namespace Genode;


/**
 * Return the managed dataspace holding the thread context area
 *
 * This function is provided by the process environment.
 */
namespace Genode {
	Rm_session  *env_context_area_rm_session();
	Ram_session *env_context_area_ram_session();
}


/******************************
 ** Thread-context allocator **
 ******************************/

Thread_base::Context *Thread_base::Context_allocator::base_to_context(addr_t base)
{
	addr_t result = base + CONTEXT_VIRTUAL_SIZE - sizeof(Context);
	return reinterpret_cast<Context *>(result);
}


addr_t Thread_base::Context_allocator::addr_to_base(void *addr)
{
	return ((addr_t)addr) & CONTEXT_VIRTUAL_BASE_MASK;
}


bool Thread_base::Context_allocator::_is_in_use(addr_t base)
{
	List_element<Thread_base> *le = _threads.first();
	for (; le; le = le->next())
		if (base_to_context(base) == le->object()->_context)
			return true;

	return false;
}


Thread_base::Context *Thread_base::Context_allocator::alloc(Thread_base *thread_base)
{
	Lock::Guard _lock_guard(_threads_lock);

	/*
	 * Find slot in context area for the new context
	 */
	addr_t base = CONTEXT_AREA_VIRTUAL_BASE;
	for (; _is_in_use(base); base += CONTEXT_VIRTUAL_SIZE) {

		/* check upper bound of context area */
		if (base >= CONTEXT_AREA_VIRTUAL_BASE + CONTEXT_AREA_VIRTUAL_SIZE)
			return 0;
	}

	_threads.insert(&thread_base->_list_element);

	return base_to_context(base);
}


void Thread_base::Context_allocator::free(Thread_base *thread_base)
{
	Lock::Guard _lock_guard(_threads_lock);

	_threads.remove(&thread_base->_list_element);
}


/*****************
 ** Thread base **
 *****************/

Thread_base::Context_allocator *Thread_base::_context_allocator()
{
	static Context_allocator context_allocator_inst;
	return &context_allocator_inst;
}


Thread_base::Context *Thread_base::_alloc_context(size_t stack_size)
{
	/*
	 * Synchronize context list when creating new threads from multiple threads
	 *
	 * XXX: remove interim fix
	 */
	static Lock alloc_lock;
	Lock::Guard _lock_guard(alloc_lock);

	/* allocate thread context */
	Context *context = _context_allocator()->alloc(this);
	if (!context)
		throw Context_alloc_failed();

	/* determine size of dataspace to allocate for context members and stack */
	enum { PAGE_SIZE_LOG2 = 12 };
	size_t ds_size = align_addr(stack_size, PAGE_SIZE_LOG2);

	if (stack_size >= CONTEXT_VIRTUAL_SIZE - sizeof(Native_utcb) - (1 << PAGE_SIZE_LOG2))
		throw Stack_too_large();

	/*
	 * Calculate base address of the stack
	 *
	 * The stack is always located at the top of the context.
	 */
	addr_t ds_addr = Context_allocator::addr_to_base(context) + CONTEXT_VIRTUAL_SIZE
	               - ds_size;

	/* add padding for UTCB if defined for the platform */
	if (sizeof(Native_utcb) >= (1 << PAGE_SIZE_LOG2))
		ds_addr -= sizeof(Native_utcb);

	/* allocate and attach backing store for the stack */
	Ram_dataspace_capability ds_cap;
	try {
		ds_cap = env_context_area_ram_session()->alloc(ds_size);
		addr_t attach_addr = ds_addr - CONTEXT_AREA_VIRTUAL_BASE;
		env_context_area_rm_session()->attach_at(ds_cap, attach_addr, ds_size);

	} catch (Ram_session::Alloc_failed) {
		throw Stack_alloc_failed();
	}

	/*
	 * Now the thread context is backed by memory, so it is safe to access its
	 * members.
	 *
	 * We need to initalize the context object's memory with zeroes,
	 * otherwise the ds_cap isn't invalid. That would cause trouble
	 * when the assignment operator of Native_capability is used.
	 */
	memset(context, 0, sizeof(Context));
	context->thread_base = this;
	context->stack_base  = ds_addr;
	context->ds_cap      = ds_cap;
	return context;
}


void Thread_base::_free_context()
{
	addr_t ds_addr = _context->stack_base - CONTEXT_AREA_VIRTUAL_BASE;
	Ram_dataspace_capability ds_cap = _context->ds_cap;
	Genode::env_context_area_rm_session()->detach((void *)ds_addr);
	Genode::env_context_area_ram_session()->free(ds_cap);
	_context_allocator()->free(this);
}


void Thread_base::name(char *dst, size_t dst_len)
{
	snprintf(dst, min(dst_len, (size_t)Context::NAME_LEN), _context->name);
}


Thread_base *Thread_base::myself() {
	using namespace Fiasco;

	return reinterpret_cast<Thread_base*>(l4_utcb_tcr()->user[UTCB_TCR_THREAD_OBJ]);
}


Thread_base::Thread_base(const char *name, size_t stack_size)
:
	_list_element(this),
	_context(_alloc_context(stack_size))
{
	strncpy(_context->name, name, sizeof(_context->name));
	_init_platform_thread();
}


Thread_base::~Thread_base()
{
	_deinit_platform_thread();
	_free_context();
}
