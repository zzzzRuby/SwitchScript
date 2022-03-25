#pragma once

#include "Joystick.h"

USB_JoystickReport_Input_t* VM_State(void);

void VM_Init(void);

void VM_Start(void);

void VM_Update(void);