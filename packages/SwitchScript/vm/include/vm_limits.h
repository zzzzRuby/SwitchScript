#pragma once

#include <stddef.h>

#define emulator_heap_size ((size_t)256)
#define emulator_exe_size ((size_t)2048)

#define atmega16u2_heap_size ((size_t)256)
#define atmega16u2_exe_size ((size_t)(512 - 2))

#define atmega32u4_heap_size ((size_t)256)
#define atmega32u4_exe_size ((size_t)(1024 - 2))

#if defined(SSVM_EMULATOR)
#   define VMHeapSize emulator_heap_size
#   define VMExeSize emulator_exe_size
#elif defined(__AVR_ATmega16U2__)
#   define VMHeapSize atmega16u2_heap_size
#   define VMExeSize atmega16u2_exe_size
#elif defined(__AVR_ATmega32U4__)
#	define VMHeapSize atmega32u4_heap_size
#	define VMExeSize atmega32u4_exe_size
#endif