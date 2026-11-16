/*
 * Address Space Layout Randomization
 * Part 1B: Prefetch
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

/*
 * Part 1
 * Find and return the single mapped address within the range [low_bound, upper_bound).
 */
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
