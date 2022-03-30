#pragma once
#include <vm_limits.h>
#include <vm.h>

#if (!defined(HeapSize) || !defined(ExeSize))
#   error Check mcu support!
#endif

#if defined(__cplusplus)
extern "C" {
#endif

uint32_t _VM_MilliSeconds(void);
void _VM_MilliSeconds_Init(void);
void _VM_MilliSeconds_Reset(void);

uint8_t _VM_ReadProgram_Byte(void);
int16_t _VM_ReadProgram_Int16(void);
uint16_t _VM_ReadProgram_UInt16(void);

typedef enum {
	Halt_None = 0,
	Halt_Stop = 1,
	Halt_Sleep = 2,
	Halt_Signal = 3
} Halt;

typedef struct {
	int16_t PC;
	int8_t Reserved : 4;
    int8_t HaltResetButtons : 1;
    int8_t Signal : 1;
	int8_t CompareResult : 2;
	Halt HaltType : 8;
    uint32_t HaltEndTime;
	JoystickState State;
	uint8_t Heap[HeapSize];
} VM;

#if defined(__cplusplus)
}
#endif