#include "../vm_internal.h"

extern VM g_VM;

#if HasEEPROM

#include <avr/eeprom.h>

static uint8_t g_Exe[ExeSize] EEMEM;

uint8_t _VM_ReadProgram_Byte(void) {
    eeprom_busy_wait();
    uint8_t result = eeprom_read_byte(&g_Exe[g_VM.PC]);
    g_VM.PC += 1;
    return result;
}

int16_t _VM_ReadProgram_Int16(void) {
    eeprom_busy_wait();
    uint16_t result = eeprom_read_word((uint16_t*)&g_Exe[g_VM.PC]);
    g_VM.PC += 2;
    return *((int16_t*)&result);
}

uint16_t _VM_ReadProgram_UInt16(void) {
    eeprom_busy_wait();
    uint16_t result = eeprom_read_word((uint16_t*)&g_Exe[g_VM.PC]);
    g_VM.PC += 2;
    return result;
}

void VM_LoadProgram(const uint8_t* buffer, uint16_t size, uint16_t offset) {
    eeprom_busy_wait();
    eeprom_update_block(buffer, &g_Exe[g_VM.PC], size);
}

static uint16_t g_ProgramLoad EEMEM = 0;

void VM_StartLoadProgram(void) { eeprom_write_word(&g_ProgramLoad, 0); }

void VM_EndLoadProgram(void) { eeprom_write_word(&g_ProgramLoad, 1); }

bool VM_IsProgramLoad(void) { return eeprom_read_word(&g_ProgramLoad) != 0; }

#else

#include <string.h>

static uint8_t g_Exe[ExeSize];

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

#endif