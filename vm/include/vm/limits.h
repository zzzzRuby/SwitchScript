#pragma once

#include <stddef.h>

#define atmega16u2_heap_size ((size_t)128)
#define atmega16u2_exe_size ((size_t)128)

#define atmega32u4_heap_size ((size_t)256)
#define atmega32u4_exe_size ((size_t)1024)

#if defined(__AVR_ATmega16U2__)
#   define HeapSize atmega16u2_heap_size
#   define ExeSize atmega16u2_exe_size
#elif defined(__AVR_ATmega32U4__)
#   define HeapSize atmega32u4_heap_size
#   define ExeSize atmega32u4_exe_size
#endif