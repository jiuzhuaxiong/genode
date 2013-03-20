/*
 * \brief  Test for IPC call via Genode's IPC framework
 * \author Norman Feske
 * \date   2009-03-26
 *
 * This program can be started as roottask replacement directly on the
 * OKL4 kernel. The main program plays the role of a server. It starts
 * a thread that acts as a client and performs an IPC call to the server.
 */

/*
 * Copyright (C) 2009-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <base/printf.h>
/* local includes */
#include "../mini_env.h"

using namespace Genode;

enum {
	LOOPS = 1000,
};

extern "C++" uint64_t rdtsc_amd64();
extern "C++" uint64_t diff_rdtsc_amd64();

inline uint64_t
inline_rdtsc_amd64() {
	uint32_t lo, hi;
	__asm__ volatile (
		"xorl %%eax,%%eax\n\t"
		"cpuid\n\t"
		:
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	__asm__ volatile (
		"rdtsc" : "=a" (lo), "=d" (hi)
	);

	return (uint64_t)hi << 32 | lo;
}

inline uint64_t
inline_diff_rdtsc_amd64() {
	uint32_t lo_1, lo_2, hi_1, hi_2;
	uint64_t begin, end;
	__asm__ volatile (
		"xorl %%eax,%%eax\n\t"
		"cpuid\n\t"
		:
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	__asm__ volatile (
		"rdtsc" : "=a" (lo_1), "=d" (hi_1)
	);
	__asm__ volatile (
		"xorl %%eax,%%eax\n\t"
		"cpuid\n\t"
		:
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	__asm__ volatile (
		"rdtsc" : "=a" (lo_2), "=d" (hi_2)
	);

	begin = (uint64_t)hi_1 << 32 | lo_1;
	end = (uint64_t)hi_2 << 32 | lo_2;

	return end-begin;
}


	uint64_t g_base1, g_base2, g_begin, g_end;

/**
 * Main program
 */
int main()
{
	uint64_t l_base1, l_base2, l_begin, l_end;
//	uint64_t l_base[LOOPS];
	uint64_t average_base = 0;

	/* three warum up passes */
	for(int i=0; i<10; i++)
		rdtsc_amd64();
	/* measure time consumed to execute 1 cpuid */
	g_begin = rdtsc_amd64();
	g_end = rdtsc_amd64();
	g_base1 = g_end-g_begin;
	g_base2 = diff_rdtsc_amd64();
	printf("uncached global vars: base1=%llu, base2=%llu\n", g_base1, g_base2);

	g_begin = rdtsc_amd64();
	g_end = rdtsc_amd64();
	g_base1 = g_end-g_begin;
	g_base2 = diff_rdtsc_amd64();
	printf("cached global vars: base1=%llu, base2=%llu\n", g_base1, g_base2);

	l_begin = rdtsc_amd64();
	l_end = rdtsc_amd64();
	l_base1 = l_end-l_begin;
	l_base2 = diff_rdtsc_amd64();
	printf("local vars normal: base1=%llu, base2=%llu\n", l_base1, l_base2);
	l_begin = inline_rdtsc_amd64();
	l_end = inline_rdtsc_amd64();
	l_base1 = l_end-l_begin;
	l_base2 = inline_diff_rdtsc_amd64();
	printf("local vars inline: base1=%llu, base2=%llu\n", l_base1, l_base2);

	for(int i=0; i<LOOPS; i++) {
		l_begin = inline_rdtsc_amd64();
		l_end = inline_rdtsc_amd64();
		average_base += l_end-l_begin;
	}
	average_base = average_base/ LOOPS;

	printf("average_base = %llu\n", average_base);

	while(1);
	return 0;
}
