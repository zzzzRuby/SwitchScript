cmake_minimum_required(VERSION 3.21)

if (NOT SWITCH_SCRIPT_BUILD_HOST)
	return()
endif()

project(SwitchScriptTools C CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)
find_package(OpenCV REQUIRED)

set(SwitchScriptClient_SOURCES
	src/ClientMain.cpp
	src/Client/MainWindow.ui src/Client/MainWindow.cpp src/Client/MainWindow.hpp)
if (WIN32)
	add_executable(SwitchScriptClient WIN32 ${SwitchScriptClient_SOURCES})
else()
	add_executable(SwitchScriptClient ${SwitchScriptClient_SOURCES})
endif()
target_link_libraries(SwitchScriptClient PRIVATE Qt6::Widgets Qt6::OpenGLWidgets opencv_world)
set_target_properties(SwitchScriptClient PROPERTIES
	AUTOUIC on
	AUTOMOC on
	AUTORCC on)

add_executable(SwitchScriptUploader src/UploaderMain.cpp)

add_library(SwitchScriptCompilerLib STATIC
	src/Compiler/Builder/ByteCodeBuilder.cpp
	src/Compiler/Builder/ByteCodeBuilder.hpp
	src/Compiler/CompilerOld.cpp
	include/compile.hpp)
target_link_libraries(SwitchScriptCompilerLib PRIVATE SwitchScriptVM)
set_target_properties(SwitchScriptCompilerLib PROPERTIES CXX_STANDARD 17)
target_include_directories(SwitchScriptCompilerLib PUBLIC include)

add_executable(SwitchScriptCompiler src/CompilerMain.cpp)
set_target_properties(SwitchScriptCompiler PROPERTIES CXX_STANDARD 17)
target_link_libraries(SwitchScriptCompiler PRIVATE SwitchScriptCompilerLib)

add_executable(SwitchScriptVMEmulator src/VMEmulatorMain.cpp)
target_link_libraries(SwitchScriptVMEmulator PRIVATE SwitchScriptVM)
set_target_properties(SwitchScriptVMEmulator PROPERTIES CXX_STANDARD 17)

#install(TARGETS
#	SwitchScriptClient
#	SwitchScriptUploader
#	SwitchScriptCompiler
#	SwitchScriptVMEmulator)