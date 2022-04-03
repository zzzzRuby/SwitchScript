cmake_minimum_required(VERSION 3.21)

if (NOT SWITCH_SCRIPT_BUILD_HOST)
	return()
endif()

project(SwitchScriptClient C CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(OpenCV REQUIRED)

set(SwitchScriptClient_SOURCES src/ClientMain.cpp)
if (WIN32)
	add_executable(SwitchScriptClient WIN32 ${SwitchScriptClient_SOURCES})
else()
	add_executable(SwitchScriptClient ${SwitchScriptClient_SOURCES})
endif()
target_link_libraries(SwitchScriptClient PRIVATE Qt6::Widgets opencv_world)
#install(TARGTS SwitchScriptClient)

add_executable(SwitchScriptUploader src/UploaderMain.cpp)
#install(TARGETS SwitchScriptUploader)

add_library(SwitchScriptCompilerLib STATIC
	src/Compiler/Builder/ByteCodeBuilder.cpp
	src/Compiler/Builder/ByteCodeBuilder.hpp
	src/Compiler/CompilerOld.cpp)
target_link_libraries(SwitchScriptCompilerLib PRIVATE SwitchScriptVM)
set_target_properties(SwitchScriptCompilerLib PROPERTIES CXX_STANDARD 17)
target_include_directories(SwitchScriptCompilerLib PUBLIC include)

add_executable(SwitchScriptCompiler src/CompilerMain.cpp)
set_target_properties(SwitchScriptCompiler PROPERTIES CXX_STANDARD 17)
target_link_libraries(SwitchScriptCompiler PRIVATE SwitchScriptCompilerLib)

#install(TARGETS SwitchScriptCompiler)