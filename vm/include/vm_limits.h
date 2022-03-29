#pragma once

#include <stddef.h>

#define emulator_heap_size ((size_t)256)
#define emulator_exe_size ((size_t)2048)

#define atmega16u2_heap_size ((size_t)16)
#define atmega16u2_exe_size ((size_t)320)

#define atmega32u4_heap_size ((size_t)256)
#define atmega32u4_exe_size ((size_t)2048)

#if defined(SSVM_EMULATOR)
#   define HeapSize emulator_heap_size
#   define ExeSize emulator_exe_size
#elif defined(__AVR_ATmega16U2__)
#   define HeapSize atmega16u2_heap_size
#   define ExeSize atmega16u2_exe_size
#elif defined(__AVR_ATmega32U4__)
#   define HeapSize atmega32u4_heap_size
#   define ExeSize atmega32u4_exe_size
#endif