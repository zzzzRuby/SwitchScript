#include "../vm_internal.h"
#include <avr/eeprom.h>

extern VM g_VM;

static uint8_t g_Exe[VMExeSize] EEMEM;

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

void VM_StartLoadProgram(void) { eeprom_busy_wait(); eeprom_write_word(&g_ProgramLoad, 0); }

void VM_EndLoadProgram(void) { eeprom_busy_wait(); eeprom_write_word(&g_ProgramLoad, 1); }

bool VM_IsProgramLoad(void) { eeprom_busy_wait(); return eeprom_read_word(&g_ProgramLoad) != 0; }