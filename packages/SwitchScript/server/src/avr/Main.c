#include <string.h>

#include <LUFA/Drivers/USB/USB.h>

#include <vm.h>

#include "Command.h"
#include "HID.h"

int main(void) {
	Command_Init();

	HID_Init();

	VM_Init();

	GlobalInterruptEnable();

	while(1) {
		Command_Update();
		
		VM_Update();

		HID_Update();
	}
}