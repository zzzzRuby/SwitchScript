cmake_minimum_required(VERSION 3.21)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(SwitchScript C)

set(SWITCH_SCRIPT_THIRD_PARTY_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
set(SWITCH_SCRIPT_THIRD_PARTY_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/third_party)

function(switch_script_require_package package_name)
    add_subdirectory(${SWITCH_SCRIPT_THIRD_PARTY_SOURCE_DIR}/${package_name} ${SWITCH_SCRIPT_THIRD_PARTY_BINARY_DIR}/${package_name} EXCLUDE_FROM_ALL)
endfunction()

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "AVR")
    set(SWITCH_SCRIPT_BUILD_AVR on)
else()
    set(SWITCH_SCRIPT_BUILD_HOST on)
endif()

add_subdirectory(vm)
add_subdirectory(tools)
add_subdirectory(server)