#include <vm.h>
#include <time.h>

static struct timespec g_VMStartTime;

void VM_MilliSeconds_Init(void) {
	(void)timespec_get(&g_VMStartTime, TIME_UTC);
}

uint32_t VM_MilliSeconds(void) {
	struct timespec now;
	(void)timespec_get(&now, TIME_UTC);

	uint64_t delta_ms = (now.tv_sec - g_VMStartTime.tv_sec) * 1000 + (now.tv_nsec - g_VMStartTime.tv_nsec) / 1000000;

	return (uint32_t)delta_ms;
}

void VM_MilliSeconds_Reset(void) {
	(void)timespec_get(&g_VMStartTime, TIME_UTC);
}