cmake_minimum_required(VERSION 3.21)

project(SwitchScriptServer C)

if (SWITCH_SCRIPT_BUILD_AVR)
	switch_script_require_package(lufa)
	
	add_executable(SwitchScriptServer src/avr/SwitchScriptServer.c src/avr/Descriptors.c src/avr/Command.c)
	target_link_libraries(SwitchScriptServer PRIVATE SwitchScriptVM LUFAUsb LUFASerial)
	set_target_properties(SwitchScriptServer PROPERTIES C_STANDARD 11)
	target_compile_definitions(SwitchScriptServer PRIVATE USE_LUFA_CONFIG_HEADER)
	target_include_directories(SwitchScriptServer PRIVATE include)
elseif (SWITCH_SCRIPT_BUILD_HOST)
	set(SWITCH_SCRIPT_ENABLE_ARCH "AVR" CACHE STRING "Architecture to build server")

	if ("AVR" IN_LIST SWITCH_SCRIPT_ENABLE_ARCH)
		set(SWITCH_SCRIPT_AVR_MCU "atmega32u4;atmega16u2" CACHE STRING "AVR MCUs")
		foreach(mcu ${SWITCH_SCRIPT_AVR_MCU})
			message(STATUS "Generating ${mcu} project")
			include(MCU/${mcu})
			execute_process(
				COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/${mcu}"
				COMMAND "${CMAKE_COMMAND}" -S "${CMAKE_CURRENT_SOURCE_DIR}/.."
					-B "${CMAKE_CURRENT_BINARY_DIR}/${mcu}"
					-DCMAKE_BUILD_TYPE=MinSizeRel
					-DCMAKE_SYSTEM_NAME=AVR
					-DCMAKE_AVR_MCU=${mcu}
					-DCMAKE_AVR_F_CPU=${F_CPU}
					-GNinja)
			message(STATUS "Generating ${mcu} project -- done")

			add_custom_target(SwitchScriptServer_${mcu} ALL COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_CURRENT_BINARY_DIR}/${mcu}")

			install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${mcu}/server/SwitchScriptServer.hex
				DESTINATION "bin/server"
				RENAME ${mcu}.hex)
		endforeach()
	endif()
endif()
