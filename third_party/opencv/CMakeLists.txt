add_definitions(-D__OPENCV_BUILD=1)

set(OPENCV_MODULE_DEFINITIONS_CONFIGMAKE "")

file(GLOB opencv_core_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/opencv/modules/core/src/*.*"
    "${CMAKE_CURRENT_SOURCE_DIR}/opencv/modules/core/src/utils/*.*")
add_library(opencv_core STATIC ${opencv_core_sources})
target_include_directories(opencv_core
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/opencv/modules/core/include"
    PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/include")
target_link_libraries(opencv_core PRIVATE zlib)
set(OPENCV_MODULE_DEFINITIONS_CONFIGMAKE "${OPENCV_MODULE_DEFINITIONS_CONFIGMAKE}#define HAVE_OPENCV_CORE\n")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/opencv/cmake/templates/opencv_modules.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/opencv2/opencv_modules.hpp")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/opencv/cmake/templates/cvconfig.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/cvconfig.h")
    
if (RUPKG_ARCHITECTURE_X86)
elseif (RUPKG_ARCHITECTURE_X64)
elseif (RUPKG_ARCHITECTURE_ARM)
elseif (RUPKG_ARCHITECTURE_ARM64)
endif()
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/opencv/cmake/templates/cv_cpu_config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/cv_cpu_config.h")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/opencv/cmake/templates/custom_hal.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/custom_hal.hpp")