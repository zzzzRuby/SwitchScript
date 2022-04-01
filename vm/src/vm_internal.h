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

void _VM_State_Init(JoystickState* state);

typedef enum {
	Halt_None = 0,
	Halt_Stop = 1,
	Halt_Sleep = 2,
	Halt_Signal = 3
} Halt;

typedef struct {
	int16_t PC;
	uint8_t Heap[HeapSize];
	JoystickState State;

    uint32_t HaltEndTime;

	int8_t Reserved : 2;
	uint8_t HaltType: 2; // HaltType
    int8_t HaltResetButtons : 1;
    int8_t Signal : 1;
	int8_t CompareResult : 2;
} VM;

#if defined(__cplusplus)
}
#endif