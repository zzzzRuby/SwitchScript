cmake_minimum_required(VERSION 3.21)

if (SWITCH_SCRIPT_BUILD_HOST)
	project(SwitchScriptUploader CXX)

	add_executable(SwitchScriptUploader src/Main.cpp)
	install(TARGETS SwitchScriptUploader)
endif()