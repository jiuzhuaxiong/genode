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


uint64_t g_begin, g_end;

uint64_t global_normal_avg_cpuid(uint64_t loops)
{
	uint64_t average_base = 0;

	printf("%llu\t",loops);
	g_begin = 0;
	g_end = 0;
	average_base = 0;
	for(uint64_t i=0; i<loops; i++) {
		g_begin = rdtsc_amd64();
		g_end = rdtsc_amd64();
		average_base += g_end-g_begin;
	}
	average_base = average_base/ loops;

	return average_base;
}

uint64_t global_inline_avg_cpuid(uint64_t loops)
{
	uint64_t average_base = 0;

	printf("%llu\t",loops);
	g_begin = 0;
	g_end = 0;
	average_base = 0;
	for(uint64_t i=0; i<loops; i++) {
		g_begin = inline_rdtsc_amd64();
		g_end = inline_rdtsc_amd64();
		average_base += g_end-g_begin;
	}
	average_base = average_base/ loops;

	return average_base;
}

uint64_t local_normal_avg_cpuid(uint64_t loops)
{
	uint64_t average_base, l_begin, l_end;

	printf("%llu\t",loops);
	l_begin = 0;
	l_end = 0;
	average_base = 0;
	for(uint64_t i=0; i<loops; i++) {
		l_begin = rdtsc_amd64();
		l_end = rdtsc_amd64();
		average_base += l_end-l_begin;
	}
	average_base = average_base/ loops;

	return average_base;
}


uint64_t local_inline_avg_cpuid(uint64_t loops)
{
	uint64_t average_base, l_begin, l_end;

	printf("%llu\t",loops);
	l_begin = 0;
	l_end = 0;
	average_base = 0;
	for(uint64_t i=0; i<loops; i++) {
		l_begin = inline_rdtsc_amd64();
		l_end = inline_rdtsc_amd64();
		average_base += l_end-l_begin;
	}
	average_base = average_base/ loops;

	return average_base;
}


/**
 * Main program
 */
int main()
{
	/* three warum up passes */
	printf("warmups: ");
	for(int i=0; i<10; i++)
		printf("%llu ", inline_rdtsc_amd64());

	printf("\n\nUsing global values and normal avg function:\n");
	printf("loops\tavg baseu\n");
	for(uint64_t i=1; i<=20000; i=(i*2))
		printf("%llu\n", global_normal_avg_cpuid(i));
	printf("done\n\n");

	printf("Using global values and inline avg function:\n");
	printf("loops\tavg baseu\n");
	for(uint64_t i=1; i<=20000; i=(i*2))
		printf("%llu\n", global_inline_avg_cpuid(i));
	printf("done\n\n");

	printf("Using local values and normal avg function:\n");
	printf("loops\tavg baseu\n");
	for(uint64_t i=1; i<=20000; i=(i*2))
		printf("%llu\n", local_normal_avg_cpuid(i));
	printf("done\n\n");

	printf("Using local values and inline avg function:\n");
	printf("loops\tavg baseu\n");
	for(uint64_t i=1; i<=20000; i=(i*2))
		printf("%llu\n", local_inline_avg_cpuid(i));
	printf("done\n");

	while(1);
	return 0;
}
