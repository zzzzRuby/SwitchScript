file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/libpng/png.h" png_version_str REGEX "^#define[ \t]+PNG_LIBPNG_VER_MAJOR[ \t]")
string(REGEX REPLACE "^#define[ \t]+PNG_LIBPNG_VER_MAJOR[ \t]+([^\"]+).*" "\\1" PNGLIB_MAJOR "${png_version_str}")
unset(png_version_str)
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/libpng/png.h" png_version_str REGEX "^#define[ \t]+PNG_LIBPNG_VER_MINOR[ \t]")
string(REGEX REPLACE "^#define[ \t]+PNG_LIBPNG_VER_MINOR[ \t]+([^\"]+).*" "\\1" PNGLIB_MINOR "${png_version_str}")
unset(png_version_str)
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/libpng/png.h" png_version_str REGEX "^#define[ \t]+PNG_LIBPNG_VER_RELEASE[ \t]")
string(REGEX REPLACE "^#define[ \t]+PNG_LIBPNG_VER_RELEASE[ \t]+([^\"]+).*" "\\1" PNGLIB_RELEASE "${png_version_str}")
unset(png_version_str)

set(PNGLIB_NAME libpng${PNGLIB_MAJOR}${PNGLIB_MINOR})
set(PNGLIB_VERSION ${PNGLIB_MAJOR}.${PNGLIB_MINOR}.${PNGLIB_RELEASE})

if (NOT EMSCRIPTEN)
  if(WIN32)
    check_symbol_exists("_M_AMD64" "" PNG_AMD64)
    check_symbol_exists("_M_ARM64" "" PNG_ARMV8)
    check_symbol_exists("_M_IX86" "" PNG_X86)
    check_symbol_exists("_M_ARM" "" PNG_ARM)
  else()
    check_symbol_exists("__x86_64__" "" PNG_AMD64)
    check_symbol_exists("__aarch64__" "" PNG_ARMV8)
    check_symbol_exists("__arm__" "" PNG_ARM)
    check_symbol_exists("__i386__" "" PNG_X86)
  endif()
endif()

if(PNG_ARMV8 OR PNG_ARM)
  set(libpng_arm_sources
    libpng/arm/arm_init.c
    libpng/arm/filter_neon_intrinsics.c
    libpng/arm/palette_neon_intrinsics.c)

  add_definitions(-DPNG_ARM_NEON_OPT=1 -DPNG_ARM_NEON_IMPLEMENTATION=1)
endif()

if(PNG_AMD64 OR PNG_X86 OR EMSCRIPTEN)
  set(libpng_intel_sources
    libpng/intel/intel_init.c
    libpng/intel/filter_sse2_intrinsics.c)
  add_definitions(-DPNG_INTEL_SSE_OPT=1)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libpng/scripts/pnglibconf.h.prebuilt
                 ${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h)

set(libpng_public_hdrs
  libpng/png.h
  libpng/pngconf.h
  "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h"
)
set(libpng_private_hdrs
  libpng/pngpriv.h
  libpng/pngdebug.h
  libpng/pnginfo.h
  libpng/pngstruct.h
)

set(libpng_sources
  ${libpng_public_hdrs}
  ${libpng_private_hdrs}
  libpng/png.c
  libpng/pngerror.c
  libpng/pngget.c
  libpng/pngmem.c
  libpng/pngpread.c
  libpng/pngrio.c
  libpng/pngrtran.c
  libpng/pngrutil.c
  libpng/pngset.c
  libpng/pngtrans.c
  libpng/pngwio.c
  libpng/pngwtran.c
  libpng/pngwutil.c
  pngread.c
  libpng/pngwrite.c
  ${libpng_arm_sources}
  ${libpng_intel_sources}
  ${libpng_mips_sources}
  ${libpng_powerpc_sources}
)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif(MSVC)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(png STATIC ${libpng_sources})
find_library(M_LIBRARY NAMES m)
if(M_LIBRARY)
  target_link_libraries(png PRIVATE ${M_LIBRARY})
endif()
target_include_directories(png INTERFACE 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libpng>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:include>)
target_compile_definitions(png INTERFACE PNG_STATIC)
target_link_libraries(png PRIVATE zlib)

target_compile_options(png PRIVATE $<$<CONFIG:Debug>:-DPNG_DEBUG>)
target_sources(png PUBLIC
  FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/libpng ${CMAKE_CURRENT_BINARY_DIR}
  FILES ${libpng_public_hdrs})