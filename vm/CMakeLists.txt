cmake_minimum_required(VERSION 3.21)

project(SwitchScriptVM C)

if (SWITCH_SCRIPT_BUILD_HOST)
    add_library(SwitchScriptVM STATIC src/vm.c src/emulator/vm_millis.c src/emulator/vm_program.c)
    target_include_directories(SwitchScriptVM PUBLIC include)
    set_target_properties(SwitchScriptVM PROPERTIES C_STANDARD 11)
    target_compile_definitions(SwitchScriptVM PUBLIC SSVM_EMULATOR)
    
    enable_language(CXX)

    add_executable(SwitchScriptVMEmulator src/emulator/main.cpp)
    target_link_libraries(SwitchScriptVMEmulator PRIVATE SwitchScriptVM)
    set_target_properties(SwitchScriptVMEmulator PROPERTIES CXX_STANDARD 17)

	#install(TARGETS SwitchScriptVMEmulator)
elseif (SWITCH_SCRIPT_BUILD_AVR)
    add_library(SwitchScriptVM OBJECT src/vm.c src/avr/vm_millis.c src/avr/vm_program.c)
    target_include_directories(SwitchScriptVM PUBLIC include)
endif()