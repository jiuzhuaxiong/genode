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

#include <sys/types.h>
#include <abi/ddi/arg.h>
#include "private/task.h"
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
 * Output character to serial port
 */
inline void serial_out_char(unsigned short comport, uint8_t c)
{
	/* wait until serial port is ready */
	while (!(inb(comport + 5) & 0x60));

	/* output character */
	outb(comport, c);
}

/**
 * Print null-terminated string to serial port
 */
inline void serial_out_string(unsigned short comport, const char *str)
{
	while (*str)
		serial_out_char(comport, *str++);
}

extern "C" void exit(int status)
{
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

extern "C" task_id_t task_get_id(void)
{
#ifdef __32_BITS
	task_id_t task_id;
	(void) __SYSCALL1(SYS_TASK_GET_ID, (sysarg_t) &task_id);

	return task_id;
#endif  /* __32_BITS__ */

#ifdef __64_BITS__
	return (task_id_t) __SYSCALL0(SYS_TASK_GET_ID);
#endif  /* __64_BITS__ */
}

extern "C" int pio_enable(void *pio_addr, size_t size, void **virt)
{
	ddi_ioarg_t	arg;

	arg.task_id = task_get_id();
	arg.ioaddr = pio_addr;
	arg.size = size;

	*virt = pio_addr;
	__SYSCALL1(SYS_IOSPACE_ENABLE, (sysarg_t) &arg);

	return 0;
}

/**
 * Main program, called by the _main() function
 */
extern "C" int main(void)
{
	char		str[128] = "\nHello, this is some code running on SPARTAN printing on the kernel console.\n";
	void		*req_port = (void*) 0x3F8, *giv_port;
	int		portsize = 8;

	klog_write(str, 77);

	pio_enable( req_port, portsize, &giv_port);

//	serial_out_string(COMPORT_0, "Hello, this is some code running on SPARTAN using an own serial implementation.\n");
	serial_out_string((unsigned short)giv_port, "Hello, this is some code running on SPARTAN using an own serial implementation.\n");

	exit(0);
}

