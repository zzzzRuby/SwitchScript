add_library(tinyxml2 STATIC tinyxml2/tinyxml2.cpp)
target_include_directories(tinyxml2 PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tinyxml2> $<INSTALL_INTERFACE:include>)
target_sources(tinyxml2 PUBLIC FILE_SET HEADERS BASE_DIRS tinyxml2 FILES tinyxml2/tinyxml2.h)