#include <vm_limits.h>
#include <vm.h>
#include <string.h>

#if (!defined(HeapSize) || !defined(ExeSize))
#   error Check mcu support!
#endif

uint32_t VM_MilliSeconds(void);
void VM_MilliSeconds_Init(void);
void VM_MilliSeconds_Reset(void);

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
    g_VM.HaltResetButtons = 0;
    g_VM.HaltEndTime = 0;
    
    VM_State_Init(&g_VM.State);

    memset(g_VM.Heap, 0, HeapSize);
}

void VM_State_Init(JoystickState* state) {
    state->Button = 0;
    state->DPad = DPadValue_None;
    state->LX = state->LY = state->RX = state->RY = 0x80;
    state->VendorSpec = 0;
}

void VM_Init(void) {
    VM_MilliSeconds_Init();
    VM_Init_Internal();
    memset(g_VM.Script, 0, ExeSize);
}

uint8_t* VM_PrepareForLoad(void) {
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
            return compareResult == 0 || compareResult == -1;
        case JumpMode_LT:
            return compareResult == -1;
        default:
            return 0;
    }
}

static uint8_t VM_ReadProgram_Byte(void) {
    uint8_t result = g_VM.Script[g_VM.PC];
    g_VM.PC += 1;
    return result;
}

static int16_t VM_ReadProgram_Int16(void) {
    int16_t result = *((int16_t*)&g_VM.Script[g_VM.PC]);
    g_VM.PC += 2;
    return result;
}

static uint16_t VM_ReadProgram_UInt16(void) {
    uint16_t result = *((uint16_t*)&g_VM.Script[g_VM.PC]);
    g_VM.PC += 2;
    return result;
}

void VM_Update(void) {
    VMCommand command = { 0 };

	switch(g_VM.HaltType)
	{
		case Halt_None:
			break;
		case Halt_Stop:
			return;
        case Halt_Sleep:
            if (g_VM.HaltEndTime >= VM_MilliSeconds())
                return;
            if (g_VM.HaltResetButtons) {
                g_VM.State.Button = 0;
                g_VM.State.DPad = DPadValue_None;
            }
            g_VM.HaltType = Halt_None;
            g_VM.HaltResetButtons = 0;
            break;
		case Halt_Signal:
            if (g_VM.Signal == 0)
                return;
            if (g_VM.HaltResetButtons) {
                g_VM.State.Button = 0;
                g_VM.State.DPad = DPadValue_None;
            }
            g_VM.HaltType = Halt_None;
            g_VM.HaltResetButtons = 0;
            break;
		default:
			break;
	}
	
	command.raw = VM_ReadProgram_Byte();

	switch(command.opCode) {
        case Op_Extern:
            switch(command.desc.externOp.externCode)
            {
                case ExternOp_Terminate:
                    g_VM.HaltType = Halt_Stop;
                    break;
                case ExternOp_Nop:
                    break;
                case ExternOp_SetStick:
                    g_VM.State.LX = VM_ReadProgram_Byte();
                    g_VM.State.LY = VM_ReadProgram_Byte();
                    g_VM.State.RX = VM_ReadProgram_Byte();
                    g_VM.State.RY = VM_ReadProgram_Byte();
                    break;
                case ExternOp_HaltUntilSignal:
                    g_VM.HaltType = Halt_Signal;
                    g_VM.Signal = 0;
                    break;
                case ExternOp_Halt:
                    g_VM.HaltType = Halt_Sleep;
                    g_VM.HaltEndTime = VM_MilliSeconds() + VM_ReadProgram_UInt16();
                    break;
                case ExternOp_Set: {
                    int16_t val = VM_ReadProgram_Int16();
                    uint8_t resultAddress = VM_ReadProgram_Byte();
                    *((int16_t*)(&g_VM.Heap[resultAddress])) = val;
                    break;
                }
                case ExternOp_ResetTimer: 
                    VM_MilliSeconds_Reset();
                    break;
                default:
                    break;
            }
            break;
        case Op_SetButton:
            if (command.desc.setButtonOp.setDPad) {
                g_VM.State.DPad = command.desc.setButtonOp.dpad;
            }
            g_VM.State.Button |= VM_ReadProgram_UInt16();
            break;
        case Op_UnsetButton:
            if (command.desc.unsetButtonOp.unsetDPad) {
                g_VM.State.DPad = DPadValue_None;
            }
            g_VM.State.Button &= ~VM_ReadProgram_UInt16();
            break;
        case Op_JumpIf:
            if (VM_ShouldJump(command.desc.jumpIfOp.mode, g_VM.CompareResult))
                g_VM.PC = VM_ReadProgram_Int16();
            else
                g_VM.PC += 2;
            break;
#define VM_LoadAluOperator() \
            int16_t aluOpA, aluOpB; \
            if (command.desc.aluOp.isLeftConst) { \
                aluOpA = VM_ReadProgram_Int16(); \
            } else { \
                uint8_t address = VM_ReadProgram_Byte(); \
                aluOpA = *((int16_t*)(&g_VM.Heap[address])); \
            } \
            if (command.desc.aluOp.isRightConst) { \
                aluOpB = VM_ReadProgram_Int16(); \
            } else { \
                uint8_t address = VM_ReadProgram_Byte(); \
                aluOpB = *((int16_t*)(&g_VM.Heap[address])); \
            }
        case Op_Add: {
            VM_LoadAluOperator();
            uint8_t resultAddress = VM_ReadProgram_Byte();
            *((int16_t*)(&g_VM.Heap[resultAddress])) = aluOpA + aluOpB;
            break;
        }
        case Op_Sub:{
            VM_LoadAluOperator();
            uint8_t resultAddress = VM_ReadProgram_Byte();
            *((int16_t*)(&g_VM.Heap[resultAddress])) = aluOpA - aluOpB;
            break;
        }
        case Op_Mul:{
            VM_LoadAluOperator();
            uint8_t resultAddress = VM_ReadProgram_Byte();
            *((int16_t*)(&g_VM.Heap[resultAddress])) = aluOpA * aluOpB;
            break;
        }
        case Op_Div:{
            VM_LoadAluOperator();
            uint8_t resultAddress = VM_ReadProgram_Byte();
            *((int16_t*)(&g_VM.Heap[resultAddress])) = aluOpA / aluOpB;
            break;
        }
        case Op_Mod:{
            VM_LoadAluOperator();
            uint8_t resultAddress = VM_ReadProgram_Byte();
            *((int16_t*)(&g_VM.Heap[resultAddress])) = aluOpA % aluOpB;
            break;
        }
        case Op_Compare: {
            VM_LoadAluOperator();
            if (aluOpA > aluOpB)
                g_VM.CompareResult = 1;
            else if (aluOpA == aluOpB)
                g_VM.CompareResult = 0;
            else if (aluOpA < aluOpB)
                g_VM.CompareResult = -1;
            break;
        }
#undef VM_LoadAluOperator
        case Op_Press:
            g_VM.State.DPad = command.desc.pressOp.dpad;
            g_VM.State.Button = VM_ReadProgram_UInt16();
            g_VM.HaltType = Halt_Sleep;
            g_VM.HaltResetButtons = 1;
            g_VM.HaltEndTime = VM_MilliSeconds() + VM_ReadProgram_UInt16();
            break;
        default:
            g_VM.PC += 1;
            break;
	}
}