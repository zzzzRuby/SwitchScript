cmake_minimum_required(VERSION 3.21)

project(SwitchScriptCompiler CXX)

add_library(SwitchScriptCompilerLib STATIC
	src/Builder/ByteCodeBuilder.cpp
	src/Builder/ByteCodeBuilder.hpp
	src/CompilerOld.cpp)
target_link_libraries(SwitchScriptCompilerLib PRIVATE SwitchScriptVM)
set_target_properties(SwitchScriptCompilerLib PROPERTIES CXX_STANDARD 17)

add_executable(SwitchScriptCompiler src/CompilerMain.cpp)
set_target_properties(SwitchScriptCompiler PROPERTIES CXX_STANDARD 17)
target_link_libraries(SwitchScriptCompiler PRIVATE SwitchScriptCompilerLib)