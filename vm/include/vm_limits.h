#pragma once

#include <stddef.h>

#define emulator_heap_size ((size_t)256)
#define emulator_exe_size ((size_t)2048)
#define emulator_has_eeprom 0

#define atmega16u2_heap_size ((size_t)256)
#define atmega16u2_exe_size ((size_t)512 - 2)
#define atmega16u2_has_eeprom 1

#define atmega32u4_mem_heap_size ((size_t)256)
#define atmega32u4_mem_exe_size ((size_t)1536)
#define atmega32u4_mem_has_eeprom 0

#define atmega32u4_heap_size ((size_t)256)
#define atmega32u4_exe_size ((size_t)1024 - 2)
#define atmega32u4_has_eeprom 1

#if defined(SSVM_EMULATOR)
#   define HeapSize emulator_heap_size
#   define ExeSize emulator_exe_size
#	define HasEEPROM emulator_has_eeprom
#elif defined(__AVR_ATmega16U2__)
#   define HeapSize atmega16u2_heap_size
#   define ExeSize atmega16u2_exe_size
#	define HasEEPROM atmega16u2_has_eeprom
#elif defined(__AVR_ATmega32U4__)
#	if !defined(SSVM_IN_MEMORY)
#		define HeapSize atmega32u4_heap_size
#		define ExeSize atmega32u4_exe_size
#		define HasEEPROM atmega32u4_has_eeprom
#	else
#		define HeapSize atmega32u4_mem_heap_size
#		define ExeSize atmega32u4_mem_exe_size
#		define HasEEPROM atmega32u4_mem_has_eeprom
#	endif
#endif