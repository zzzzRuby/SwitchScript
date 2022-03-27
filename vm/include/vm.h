#pragma once

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
	uint16_t Button;
	uint8_t  DPad;
	uint8_t  LX;
	uint8_t  LY;
	uint8_t  RX;
	uint8_t  RY;
	uint8_t  VendorSpec;
} JoystickState;

typedef enum {
	ButtonValue_Y       = 0x01,
	ButtonValue_B       = 0x02,
	ButtonValue_A       = 0x04,
	ButtonValue_X       = 0x08,
	ButtonValue_L       = 0x10,
	ButtonValue_R       = 0x20,
	ButtonValue_ZL      = 0x40,
	ButtonValue_ZR      = 0x80,
	ButtonValue_Select  = 0x100,
	ButtonValue_Start   = 0x200,
	ButtonValue_LClick  = 0x400,
	ButtonValue_RClick  = 0x800,
	ButtonValue_Home    = 0x1000,
	ButtonValue_Capture = 0x2000,
} ButtonValue;

typedef enum {
	DPadValue_Top          = 0,
	DPadValue_TopRight     = 1,
	DPadValue_Right        = 2,
	DPadValue_BottomRight  = 3,
	DPadValue_Bottom       = 4,
	DPadValue_BottomLeft   = 5,
	DPadValue_Left         = 6,
	DPadValue_TopLeft      = 7
} DPadValue;

#define DPadValue_None 8

typedef enum {
    JumpMode_Always,
    JumpMode_GT,
    JumpMode_GE,
    JumpMode_EQ,
    JumpMode_LE,
    JumpMode_LT
} JumpMode;

typedef enum {
	Op_Terminate = 0,
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
	Op_Nop = 13,
	Op_Reversed = 14,
	Op_Extern = 15,
} Op;

typedef enum {
    ExternOp_Nop
} ExternOp;

typedef union {
	uint8_t raw;
	union {
		struct {
			uint8_t opCode : 4; // Op
			int8_t rawDesc : 4;
		};
		union {
			struct {
				uint8_t opCode : 4; // Op
				uint8_t dpad : 3; // DPadValue
				int8_t setDPad : 1;
			} setButtonOp;

			struct {
				uint8_t opCode : 4; // Op
				int8_t reserved : 3;
				int8_t unsetDPad : 1;
			} unsetButtonOp;

			struct {
				uint8_t opCode : 4; // Op
				uint8_t mode : 4; // JumpMode
			} jumpIfOp;

			struct {
				uint8_t opCode : 4; // Op
				int8_t isLeftConst : 1;
				int8_t isRightConst : 1;
				int8_t reserved : 2;
			} aluOp;

			struct {
				uint8_t opCode : 4; // Op
				uint8_t externCode : 4; // ExternOp
			} externOp;
		} desc;
	};
} VMCommand;

JoystickState* VM_State(void);

void VM_Init(void);

uint8_t* VM_InitForLoad(void);

void VM_Start(void);

void VM_Stop(void);

void VM_Update(void);

void VM_Signal(void);

int8_t VM_IsTerminated(void);

#if defined(__cplusplus)
}
#endif