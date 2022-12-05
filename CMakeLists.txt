set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" ${CMAKE_MODULE_PATH})

cmake_minimum_required(VERSION 3.23)
project(SwitchScriptProject C CXX)

include(Rupkg)

rupkg_run()