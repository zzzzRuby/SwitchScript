rupkg_start_project(PROJECT_NAME SwitchScript)

if (RUPKG_PLATFORM_AVR)
    set(SWITCH_SCRIPT_BUILD_AVR on)
else()
    set(SWITCH_SCRIPT_BUILD_HOST on)
endif()

add_subdirectory(vm)
add_subdirectory(tools)
add_subdirectory(server)

rupkg_end_project()