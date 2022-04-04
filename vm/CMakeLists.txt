cmake_minimum_required(VERSION 3.21)

project(SwitchScriptVM C)

set(SwitchScriptVM_SOURCES src/vm.c src/vm_internal.h include/vm.h include/vm_limits.h)

if (SWITCH_SCRIPT_BUILD_HOST)
    add_library(SwitchScriptVM STATIC ${SwitchScriptVM_SOURCES} src/emulator/vm_millis.c src/emulator/vm_program.c)
    target_include_directories(SwitchScriptVM PUBLIC include)
    set_target_properties(SwitchScriptVM PROPERTIES C_STANDARD 11)
    target_compile_definitions(SwitchScriptVM PUBLIC SSVM_EMULATOR)
elseif (SWITCH_SCRIPT_BUILD_AVR)
    add_library(SwitchScriptVM OBJECT ${SwitchScriptVM_SOURCES} src/avr/vm_millis.c src/avr/vm_program.c)
    target_include_directories(SwitchScriptVM PUBLIC include)
endif()