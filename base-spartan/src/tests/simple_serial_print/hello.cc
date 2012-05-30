/*
 * \brief  Simple roottask replacement for OKL4 that just prints some text
 * \author Norman Feske
 * \date   2008-09-01
 */

/*
 * Copyright (C) 2008-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

//#include "private/libc.h"
//#include "private/async.h"
#include <sys/types.h>
#include <abi/ddi/arg.h>
#include "private/syscall.h"
#include "private/malloc.h"
#include "private/io.h"

/**
 * Read byte from I/O port
 */
inline uint8_t inb(unsigned short port)
{
	uint8_t res;
	asm volatile ("inb %%dx, %0" :"=a"(res) :"Nd"(port));
	return res;
}

/**
 * Write byte to I/O port
 */
inline void outb(unsigned short port, uint8_t val)
{
	asm volatile ("outb %b0, %w1" : : "a" (val), "Nd" (port));
}

/**
 * Definitions of PC serial ports
 */
enum Comport { COMPORT_0, COMPORT_1, COMPORT_2, COMPORT_3 };


/**
 * Output character to serial port
 */
inline void serial_out_char(Comport comport, uint8_t c)
{
	static int io_port[] = { 0x3f8, 0x2f8, 0x3e8, 0x2e8 };
	/* wait until serial port is ready */
	while (!(inb(io_port[comport] + 5) & 0x60));

	/* output character */
	outb(io_port[comport], c);
}


/**
 * Print null-terminated string to serial port
 */
inline void serial_out_string(Comport comport, const char *str)
{
	while (*str)
		serial_out_char(comport, *str++);
}

extern "C" void exit(int status)
{
/*
	if (env_setup) {
		__stdio_done();
		task_retval(status);
		fibril_teardown(__tcb_get()->fibril_data);
	}
*/
	__SYSCALL1(SYS_TASK_EXIT, false);
	/* Unreachable */
	while (1);
}

extern "C" size_t klog_write(const void *buf, size_t size)
{
	ssize_t ret = (ssize_t) __SYSCALL3(SYS_KLOG, 1, (sysarg_t) buf, size);
	
	if (ret >= 0)
		return (size_t) ret;
	
	return 0;
}

/**
 * Main program, called by the startup code in crt0.s
 */
extern "C" int _main(void)
{
	char str[20] = "\nTest succefull !\n";
	ddi_ioarg_t arg;

	arg.task_id = 1;
	arg.ioaddr = (void*) 0x3F8;
	arg.size = 8;

//	__malloc_init();
//	__async_init();
	
/*
	fibril_t *fibril = fibril_setup();
	if (fibril == NULL)
		abort();
	
	__tcb_set(fibril->tcb);
	__pcb = (pcb_t *) pcb_ptr;

#ifdef __IN_SHARED_LIBC__
	if (__pcb != NULL && __pcb->rtld_runtime != NULL) {
		runtime_env = (runtime_env_t *) __pcb->rtld_runtime;
	}
#endif
*/
	/*
	 * Get command line arguments and initialize
	 * standard input and output
	 */
/*
	if (__pcb == NULL) {
		argc = 0;
		argv = NULL;
		__stdio_init(0);
	} else {
		argc = __pcb->argc;
		argv = __pcb->argv;
		__stdio_init(__pcb->filc);
		(void) chdir(__pcb->cwd);
	}
*/
//	__stdio_init(0);

	klog_write(str, 18);

	__SYSCALL1(SYS_IOSPACE_ENABLE, (sysarg_t) &arg);

	serial_out_string(COMPORT_0, "Hallo, this is some code running on OKL4.\n");
	serial_out_string(COMPORT_0, "Returning from main...\n");

	exit(0);
}

