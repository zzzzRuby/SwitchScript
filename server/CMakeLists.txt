cmake_minimum_required(VERSION 3.21)

project(SwitchScriptServer C)

add_subdirectory(../vm vm)

add_executable(SwitchScriptServer src/SwitchScriptServer.c src/Descriptors.c)

target_link_libraries(SwitchScriptServer PRIVATE SwitchScriptVM)