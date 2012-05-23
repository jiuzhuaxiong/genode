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

//#include <stdio.h>

extern "C" int printf(const char *, ...);
//	PRINTF_ATTRIBUTE(1, 2);

//int main(void)
extern "C" int main(void)
{
	printf("\nHello HelenOS, this is Genode :-)\n\n");
	return 0;
}
