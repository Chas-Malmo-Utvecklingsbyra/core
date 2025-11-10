#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include <stdint.h>

#include "clock_monotonic.h"

/* #define CLOCK_MONOTONIC 1 */
uint64_t SystemMonotonicMS()
{
	long            ms;
	time_t          s;

	struct timespec spec;
	clock_gettime(CLOCK_MONOTONIC, &spec);
	s  = spec.tv_sec;
	ms = (spec.tv_nsec / 1000000);

	uint64_t result = s;
	result *= 1000;
	result += ms;

	return result;
}