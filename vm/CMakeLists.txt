cmake_minimum_required(VERSION 3.21)

project(SwitchScriptVM C)

add_library(SwitchScriptVM STATIC src/vm.c)
target_include_directories(SwitchScriptVM PUBLIC include)
set_target_properties(SwitchScriptVM PROPERTIES C_STANDARD 11)

if (SWITCH_SCRIPT_BUILD_HOST)
    enable_language(CXX)

    target_sources(SwitchScriptVM PRIVATE src/emulator/vm_millis.c)
    target_compile_definitions(SwitchScriptVM PUBLIC SSVM_EMULATOR)

    add_executable(SwitchScriptVMEmulator src/emulator/main.cpp)

    target_link_libraries(SwitchScriptVMEmulator PRIVATE SwitchScriptVM)
    set_target_properties(SwitchScriptVMEmulator PROPERTIES CXX_STANDARD 17)
elseif (SWITCH_SCRIPT_BUILD_AVR)
    target_sources(SwitchScriptVM PRIVATE src/avr/vm_millis.c)
endif()