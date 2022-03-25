#include <vm/limits.h>
#include <stdint.h>

#include "VM.h"

#if !defined(HeapSize) || !defined(ExeSize)
#   error "Check mcu support!"
#endif

typedef enum
{
	Op_Nop = 0,
	Op_SetButton = 1,
	Op_UnsetButton = 2,
	Op_JumpIf = 3,
	Op_Add = 4,
	Op_Sub = 5,
	Op_Mul = 6,
	Op_Div = 7,
	Op_Mod = 8,
	Op_Compare = 9,
	Op_SetStick = 10,
	Op_HaltUntilSignal = 11,
	Op_Halt = 12,
	Op_Reversed0 = 13,
	Op_Reversed1 = 14,
	Op_Extern = 15,
} Op;

typedef union {
	uint8_t Raw;
	struct {
		Op OpCode : 4;
		union {
			struct {
				int8_t setDPad : 1;
				int8_t dpad : 3;
			} setButton;

			struct {
				int8_t unsetDPad : 1;
			} unsetButton;

			struct {
				int8_t is16Bit : 1;
				int8_t isLeftConst : 1;
				int8_t isRightConst : 1;
			} alu;
		};
	};
} SSVM_Command;

typedef enum {
	Halt_None = 0,
	Halt_Stop = 1,
	Halt_Sleep = 2,
	Halt_Signal = 3
} Halt;

typedef struct {
	int16_t PC;
	int8_t Reserved : 6;
	int8_t CompareResult : 2;
	int8_t HaltType;
	uint32_t VMStartTime;
	uint32_t HaltStartTime;
	USB_JoystickReport_Input_t State;
	uint8_t Heap[HeapSize];
	uint8_t Script[ExeSize];
} VM;

static VM g_VM;

USB_JoystickReport_Input_t* VM_State(void) {
    return &g_VM.State;
}

void VM_Init(void) {
	g_VM.HaltType = Halt_Stop;
}

void VM_Start(void) {
	g_VM.HaltType = Halt_None;
}

void VM_Update(void) {
	SSVM_Command command;
	int16_t currentPtr;

	switch(g_VM.HaltType)
	{
		case Halt_None:
			break;
		case Halt_Stop:
			return;
		case Halt_Sleep:
			//check time;
		case Halt_Signal:
			//check signal;
		default:
			break;
	}
	
	currentPtr = g_VM.PC;
	command.Raw = g_VM.Script[currentPtr];
	currentPtr++;

	switch(command.OpCode)
	{
	case Op_Nop:
		break;
	case Op_SetButton:
		break;
	case Op_UnsetButton:
		break;
	case Op_JumpIf:
		break;
	case Op_Add:
		break;
	case Op_Sub:
		break;
	case Op_Mul:
		break;
	case Op_Div:
		break;
	case Op_Mod:
		break;
	case Op_Compare:
		break;
	case Op_SetStick:
		break;
	case Op_HaltUntilSignal:
		break;
	case Op_Halt:
		break;
	case Op_Reversed0:
		break;
	case Op_Reversed1:
		break;
	case Op_Extern:
		break;
	default:
		break;
	}

	g_VM.PC = currentPtr;
}