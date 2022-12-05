if (RUPKG_PLATFORM_AVR)
	add_executable(SwitchScriptServer src/avr/Main.c
		src/avr/LUFAConfig.h
		src/avr/Descriptors.c src/avr/Descriptors.h
		src/avr/Command.c src/avr/Command.h
		src/avr/HID.c src/avr/HID.h)
	target_link_libraries(SwitchScriptServer PRIVATE SwitchScriptVM LUFAUsb LUFASerial)
	set_target_properties(SwitchScriptServer PROPERTIES
		C_STANDARD 11
		OUTPUT_NAME "SwitchScriptServer_${CMAKE_AVR_MCU}")
	target_compile_definitions(SwitchScriptServer PRIVATE USE_LUFA_CONFIG_HEADER)
	target_include_directories(SwitchScriptServer PRIVATE src/avr)

	rupkg_mark_as_tool(SwitchScriptServer)
	rupkg_install(TARGETS SwitchScriptServer
		RUNTIME DESTINATION "bin/${RUPKG_HOST_TRIPLE}"
		CONFIGURATIONS Release)
endif()
