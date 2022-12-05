#include "../vm_internal.h"
#include <string.h>

static uint8_t g_Exe[VMExeSize];
extern VM g_VM;

uint8_t _VM_ReadProgram_Byte(void) {
    uint8_t result = g_Exe[g_VM.PC];
    g_VM.PC += 1;
    return result;
}

int16_t _VM_ReadProgram_Int16(void) {
    int16_t result = *((int16_t*)&g_Exe[g_VM.PC]);
    g_VM.PC += 2;
    return result;
}

uint16_t _VM_ReadProgram_UInt16(void) {
    uint16_t result = *((uint16_t*)&g_Exe[g_VM.PC]);
    g_VM.PC += 2;
    return result;
}

void VM_LoadProgram(const uint8_t* buffer, uint16_t size, uint16_t offset) {
    memcpy(&g_Exe[offset], buffer, size);
}

static bool g_ProgramLoad = false;

void VM_StartLoadProgram(void) { g_ProgramLoad = false; }

void VM_EndLoadProgram(void) { g_ProgramLoad = true; }

bool VM_IsProgramLoad(void) { return g_ProgramLoad; } 