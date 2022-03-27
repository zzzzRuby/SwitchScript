#include <vm_limits.h>
#include <vm.h>
#include <string.h>

#if (!defined(HeapSize) || !defined(ExeSize))
#   error "Check mcu support!"
#endif

uint32_t VM_MilliSeconds(void);
void VM_MilliSeconds_Init(void);

typedef enum {
	Halt_None = 0,
	Halt_Stop = 1,
	Halt_Sleep = 2,
	Halt_Signal = 3
} Halt;

typedef struct {
	int16_t PC;
	int8_t Reserved : 5;
    int8_t Signal : 1;
	int8_t CompareResult : 2;
	Halt HaltType : 8;
    uint16_t HaltEndTime;
	JoystickState State;
	uint8_t Heap[HeapSize];
	uint8_t Script[ExeSize];
} VM;

static VM g_VM;

JoystickState* VM_State(void) {
    return &g_VM.State;
}

static void VM_Init_Internal(void) {
    g_VM.PC = 0;
	g_VM.HaltType = Halt_Stop;
    g_VM.Signal = 0;
    g_VM.CompareResult = 0;
    g_VM.HaltEndTime = 0;
    
	g_VM.State.Button = 0;
	g_VM.State.DPad = DPadValue_None;
	g_VM.State.LX = g_VM.State.LY = g_VM.State.RX = g_VM.State.RY = 0x80;
    g_VM.State.VendorSpec = 0;

    memset(g_VM.Heap, 0, HeapSize);
}

void VM_Init(void) {
    VM_MilliSeconds_Init();
    VM_Init_Internal();
    memset(g_VM.Script, 0, ExeSize);
}

uint8_t* VM_InitForLoad(void) {
    VM_Init_Internal();
    return g_VM.Script;
}

void VM_Start(void) {
	g_VM.HaltType = Halt_None;
}

void VM_Stop(void) {
	g_VM.HaltType = Halt_Stop;
}

void VM_Signal(void) {
    g_VM.Signal = 1;
}

int8_t VM_IsTerminated(void) {
    return g_VM.HaltType == Halt_Stop;
}

static int8_t VM_ShouldJump(JumpMode mode, int8_t compareResult) {
    switch(mode) {
        case JumpMode_Always:
            return 1;
        case JumpMode_GT:
            return compareResult == 1;
        case JumpMode_GE:
            return compareResult == 1 || compareResult == 0;
        case JumpMode_EQ:
            return compareResult == 0;
        case JumpMode_LE:
            return compareResult == 0 || compareResult == 2;
        case JumpMode_LT:
            return compareResult == 2;
        default:
            return 0;
    }
}

void VM_Update(void) {
    VMCommand command = { 0 };
	int16_t currentPtr;

    _Static_assert(sizeof(VMCommand) == 1, "check VMCommand size");

	switch(g_VM.HaltType)
	{
		case Halt_None:
			break;
		case Halt_Stop:
			return;
		case Halt_Sleep:
            if (g_VM.HaltEndTime < VM_MilliSeconds())
                return;
            g_VM.HaltType = Halt_None;
            break;
		case Halt_Signal:
            if (g_VM.Signal == 0)
                return;
            g_VM.HaltType = Halt_None;
            break;
		default:
			break;
	}
	
	currentPtr = g_VM.PC;
	command.raw = g_VM.Script[currentPtr];

	switch(command.opCode) {
        case Op_Terminate:
            g_VM.HaltType = Halt_Stop;
            currentPtr += 1;
            break;
        case Op_SetButton:
            if (command.desc.setButtonOp.setDPad) {
                g_VM.State.DPad = command.desc.setButtonOp.dpad;
            }
            g_VM.State.Button |= (((uint16_t)g_VM.Script[currentPtr + 1]) & ((uint16_t)g_VM.Script[currentPtr + 2] << 8));
            currentPtr += 3;
            break;
        case Op_UnsetButton:
            if (command.desc.unsetButtonOp.unsetDPad) {
                g_VM.State.DPad = DPadValue_None;
            }
            g_VM.State.Button &= ~(((uint16_t)g_VM.Script[currentPtr + 1]) & ((uint16_t)g_VM.Script[currentPtr + 2] << 8));
            currentPtr += 3;
            break;
        case Op_JumpIf:
            if (VM_ShouldJump(command.desc.jumpIfOp.mode, g_VM.CompareResult))
                currentPtr = (((uint16_t)g_VM.Script[currentPtr + 1]) & ((uint16_t)g_VM.Script[currentPtr + 2] << 8));
            else
                currentPtr += 1;
            break;

#define VM_LoadAluOperator() \
            int16_t aluOpA, aluOpB; \
            if (command.desc.aluOp.isLeftConst) { \
                aluOpA = *((int16_t*)&g_VM.Script[currentPtr + 1]); \
                currentPtr += 2; \
            } else { \
                uint8_t address = g_VM.Script[currentPtr + 1]; \
                aluOpA = *((int16_t*)(&g_VM.Heap[address])); \
                currentPtr += 1; \
            } \
            if (command.desc.aluOp.isRightConst) { \
                aluOpB = *((int16_t*)&g_VM.Script[currentPtr + 1]); \
                currentPtr += 2; \
            } else { \
                uint8_t address = g_VM.Script[currentPtr + 1]; \
                aluOpB = *((int16_t*)(&g_VM.Heap[address])); \
                currentPtr += 1; \
            }
        case Op_Add: {
            VM_LoadAluOperator();
            uint8_t resultAddress = g_VM.Script[currentPtr + 1];
            *((int16_t*)(&g_VM.Heap[resultAddress])) = aluOpA + aluOpB;
            currentPtr += 2;
            break;
        }
        case Op_Sub:{
            VM_LoadAluOperator();
            uint8_t resultAddress = g_VM.Script[currentPtr + 1];
            *((int16_t*)(&g_VM.Heap[resultAddress])) = aluOpA - aluOpB;
            currentPtr += 2;
            break;
        }
        case Op_Mul:{
            VM_LoadAluOperator();
            uint8_t resultAddress = g_VM.Script[currentPtr + 1];
            *((int16_t*)(&g_VM.Heap[resultAddress])) = aluOpA * aluOpB;
            currentPtr += 2;
            break;
        }
        case Op_Div:{
            VM_LoadAluOperator();
            uint8_t resultAddress = g_VM.Script[currentPtr + 1];
            *((int16_t*)(&g_VM.Heap[resultAddress])) = aluOpA / aluOpB;
            currentPtr += 2;
            break;
        }
        case Op_Mod:{
            VM_LoadAluOperator();
            uint8_t resultAddress = g_VM.Script[currentPtr + 1];
            *((int16_t*)(&g_VM.Heap[resultAddress])) = aluOpA % aluOpB;
            currentPtr += 2;
            break;
        }
        case Op_Compare: {
            VM_LoadAluOperator();
            if (aluOpA > aluOpB)
                g_VM.CompareResult = 1;
            else if (aluOpA == aluOpB)
                g_VM.CompareResult = 0;
            else if (aluOpA < aluOpB)
                g_VM.CompareResult = 2;
            currentPtr += 1;
            break;
        }
#undef VM_LoadAluOperator

        case Op_SetStick:
            g_VM.State.LX = g_VM.Script[currentPtr + 1];
            g_VM.State.LY = g_VM.Script[currentPtr + 2];
            g_VM.State.RX = g_VM.Script[currentPtr + 3];
            g_VM.State.RY = g_VM.Script[currentPtr + 4];
            currentPtr += 5;
            break;
        case Op_HaltUntilSignal:
            g_VM.HaltType = Halt_Signal;
            g_VM.Signal = 0;
            currentPtr += 1;
            break;
        case Op_Halt:
            g_VM.HaltType = Halt_Sleep;
            g_VM.HaltEndTime = VM_MilliSeconds() + (((uint16_t)g_VM.Script[currentPtr + 1]) & ((uint16_t)g_VM.Script[currentPtr + 2] << 8));
            currentPtr += 3;
            break;
        case Op_Nop:
            currentPtr += 1;
            break;
        case Op_Reversed:
            currentPtr += 1;
            break;
        case Op_Extern:
            currentPtr += 1;
            break;
        default:
            currentPtr += 1;
            break;
	}

	g_VM.PC = currentPtr;
}