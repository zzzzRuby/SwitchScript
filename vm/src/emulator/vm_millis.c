#include "../vm_internal.h"
#include <time.h>

static struct timespec g_VMStartTime;

void _VM_MilliSeconds_Init(void) {
	(void)timespec_get(&g_VMStartTime, TIME_UTC);
}

uint32_t _VM_MilliSeconds(void) {
	struct timespec now;
	(void)timespec_get(&now, TIME_UTC);

	uint64_t delta_ms = (now.tv_sec - g_VMStartTime.tv_sec) * 1000 + (now.tv_nsec - g_VMStartTime.tv_nsec) / 1000000;

	return (uint32_t)delta_ms;
}

void _VM_MilliSeconds_Reset(void) {
	(void)timespec_get(&g_VMStartTime, TIME_UTC);
}