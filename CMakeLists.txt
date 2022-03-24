cmake_minimum_required(VERSION 3.21)

project(SwitchScript C CXX)

set(SWITCH_SCRIPT_THIRD_PARTY_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
set(SWITCH_SCRIPT_THIRD_PARTY_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/third_party)

function(switch_script_require_package package_name)
    add_subdirectory(${SWITCH_SCRIPT_THIRD_PARTY_SOURCE_DIR}/${package_name} ${SWITCH_SCRIPT_THIRD_PARTY_BINARY_DIR}/${package_name})
endfunction()

add_subdirectory(client)