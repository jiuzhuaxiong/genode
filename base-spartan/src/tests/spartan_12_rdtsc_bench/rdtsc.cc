#include <base/stdint.h>

using namespace Genode;
/*
uint64_t
rdtsc_i386()
{
	uint64_t t;
        __asm__ __volatile__ (
                        "pushl %%ebx\n\t"
                        "xorl %%eax,%%eax\n\t"
                        "cpuid\n\t"
                        "popl %%ebx\n\t"
                        :
                        :
                        : "%eax", "%ecx", "%edx"
                        );
        __asm__ __volatile__ (
                        "rdtsc" : "=A" (t)
                        );

        return t;
}
*/
/*
uint64_t rdtsc_i386_amd64(void)
{
	uint32_t a, d;

	__asm__ volatile("rdtsc" : "=a" (a), "=d" (d));

	return ((uint64_t)a) | (((uint64_t)d) << 32);;
}
*/

uint64_t
rdtsc_amd64() {
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

uint64_t
diff_rdtsc_amd64() {
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

