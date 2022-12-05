file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/double-conversion/double-conversion/*.cc)
add_library(double-conversion STATIC ${sources})
target_include_directories(
    double-conversion PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/double-conversion>
        $<INSTALL_INTERFACE:include>)
target_sources(double-conversion PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/double-conversion
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/double-conversion/double-conversion/double-conversion.h
        ${CMAKE_CURRENT_SOURCE_DIR}/double-conversion/double-conversion/string-to-double.h
        ${CMAKE_CURRENT_SOURCE_DIR}/double-conversion/double-conversion/double-to-string.h
        ${CMAKE_CURRENT_SOURCE_DIR}/double-conversion/double-conversion/utils.h)