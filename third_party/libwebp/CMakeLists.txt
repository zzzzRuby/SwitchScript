if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif()

project(WebP C)

set(WEBP_ENABLE_SIMD on)
set(WEBP_NEAR_LOSSLESS ON)
set(WEBP_ENABLE_SWAP_16BIT_CSP OFF)
set(WEBP_BITTRACE "0")

set(WEBP_DEP_LIBRARIES)

include(CheckCSourceCompiles)
include(CMakePushCheckState)
include(CheckIncludeFiles)

check_c_source_compiles("
    int main(void) {
      (void)__builtin_bswap16(0);
      return 0;
    }
  " HAVE_BUILTIN_BSWAP16)
check_c_source_compiles("
    int main(void) {
      (void)__builtin_bswap32(0);
      return 0;
    }
  " HAVE_BUILTIN_BSWAP32)
check_c_source_compiles("
    int main(void) {
      (void)__builtin_bswap64(0);
      return 0;
    }
  " HAVE_BUILTIN_BSWAP64)

find_package(Threads)
if(Threads_FOUND)
  list(APPEND WEBP_DEP_LIBRARIES Threads::Threads)
endif()
set(WEBP_USE_THREAD ${Threads_FOUND})
set(WEBP_HAVE_GL off)

check_c_source_compiles("
    #include <math.h>
    int main(int argc, char** argv) {
      return (int)pow(argc, 2.5);
    }
  " HAVE_MATH_LIBRARY)
if(NOT HAVE_MATH_LIBRARY)
  list(APPEND WEBP_DEP_LIBRARIES m)
endif()

check_include_files("stdlib.h;stdarg.h;string.h;float.h" STDC_HEADERS)
check_include_files(dlfcn.h HAVE_DLFCN_H)
check_include_files(GLUT/glut.h HAVE_GLUT_GLUT_H)
check_include_files(GL/glut.h HAVE_GL_GLUT_H)
check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(memory.h HAVE_MEMORY_H)
check_include_files(OpenGL/glut.h HAVE_OPENGL_GLUT_H)
check_include_files(shlwapi.h HAVE_SHLWAPI_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(wincodec.h HAVE_WINCODEC_H)
check_include_files(windows.h HAVE_WINDOWS_H)

if(HAVE_WINCODEC_H)
  list(APPEND WEBP_DEP_LIBRARIES
              shlwapi
              ole32
              windowscodecs)
endif()

function(webp_check_compiler_flag WEBP_SIMD_FLAG ENABLE_SIMD)
  if(NOT ENABLE_SIMD)
    message(STATUS "Disabling ${WEBP_SIMD_FLAG} optimization.")
    set(WEBP_HAVE_${WEBP_SIMD_FLAG} 0 PARENT_SCOPE)
    return()
  endif()
  unset(WEBP_HAVE_FLAG_${WEBP_SIMD_FLAG} CACHE)
  cmake_push_check_state()
  set(CMAKE_REQUIRED_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/libwebp)
  check_c_source_compiles("
      #include \"${CMAKE_CURRENT_SOURCE_DIR}/libwebp/src/dsp/dsp.h\"
      int main(void) {
        #if !defined(WEBP_USE_${WEBP_SIMD_FLAG})
        this is not valid code
        #endif
        return 0;
      }
    " WEBP_HAVE_FLAG_${WEBP_SIMD_FLAG})
  cmake_pop_check_state()
  if(WEBP_HAVE_FLAG_${WEBP_SIMD_FLAG})
    set(WEBP_HAVE_${WEBP_SIMD_FLAG} 1 PARENT_SCOPE)
  else()
    set(WEBP_HAVE_${WEBP_SIMD_FLAG} 0 PARENT_SCOPE)
  endif()
endfunction()

set(WEBP_SIMD_FLAGS "SSE41;SSE2;MIPS32;MIPS_DSP_R2;NEON;MSA")
set(WEBP_SIMD_FILE_EXTENSIONS
    "_sse41.c;_sse2.c;_mips32.c;_mips_dsp_r2.c;_neon.c;_msa.c")
if(MSVC AND CMAKE_C_COMPILER_ID STREQUAL "MSVC")
  if(MSVC_VERSION GREATER_EQUAL 1800 AND NOT CMAKE_C_FLAGS MATCHES "/arch:")
    set(SIMD_ENABLE_FLAGS)
  else()
    set(SIMD_ENABLE_FLAGS "/arch:AVX;/arch:SSE2;;;;")
  endif()
  set(SIMD_DISABLE_FLAGS)
else()
  set(SIMD_ENABLE_FLAGS
      "-msse4.1;-msse2;-mips32;-mdspr2;-mfpu=neon;-mmsa")
  set(SIMD_DISABLE_FLAGS
      "-mno-sse4.1;-mno-sse2;;-mno-dspr2;;-mno-msa")
endif()

set(WEBP_SIMD_FILES_TO_NOT_INCLUDE)
set(WEBP_SIMD_FILES_TO_INCLUDE)
set(WEBP_SIMD_FLAGS_TO_INCLUDE)

if(ANDROID)
  if("${CMAKE_ANDROID_ARCH_ABI}" STREQUAL "armeabi-v7a")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=neon ")
  endif()
endif()

list(LENGTH WEBP_SIMD_FLAGS WEBP_SIMD_FLAGS_LENGTH)
math(EXPR WEBP_SIMD_FLAGS_RANGE "${WEBP_SIMD_FLAGS_LENGTH} - 1")

foreach(I_SIMD RANGE ${WEBP_SIMD_FLAGS_RANGE})
  if(EMSCRIPTEN AND ${I_SIMD} GREATER_EQUAL 2)
    break()
  endif()

  list(GET WEBP_SIMD_FLAGS ${I_SIMD} WEBP_SIMD_FLAG)

  unset(WEBP_HAVE_${WEBP_SIMD_FLAG} CACHE)
  cmake_push_check_state()
  set(CMAKE_REQUIRED_FLAGS)
  webp_check_compiler_flag(${WEBP_SIMD_FLAG} ${WEBP_ENABLE_SIMD})
  if(NOT WEBP_HAVE_${WEBP_SIMD_FLAG})
    list(GET SIMD_ENABLE_FLAGS ${I_SIMD} SIMD_COMPILE_FLAG)
    if(EMSCRIPTEN)
      set(SIMD_COMPILE_FLAG "-msimd128 ${SIMD_COMPILE_FLAG}")
    endif()
    set(CMAKE_REQUIRED_FLAGS ${SIMD_COMPILE_FLAG})
    webp_check_compiler_flag(${WEBP_SIMD_FLAG} ${WEBP_ENABLE_SIMD})
  else()
    if(MSVC AND SIMD_ENABLE_FLAGS)
      list(GET SIMD_ENABLE_FLAGS ${I_SIMD} SIMD_COMPILE_FLAG)
    else()
      set(SIMD_COMPILE_FLAG " ")
    endif()
  endif()
  list(GET WEBP_SIMD_FILE_EXTENSIONS ${I_SIMD} WEBP_SIMD_FILE_EXTENSION)
  file(GLOB SIMD_FILES "${CMAKE_CURRENT_SOURCE_DIR}/libwebp/"
            "src/dsp/*${WEBP_SIMD_FILE_EXTENSION}")
  if(WEBP_HAVE_${WEBP_SIMD_FLAG})
    # Memorize the file and flags.
    foreach(FILE ${SIMD_FILES})
      list(APPEND WEBP_SIMD_FILES_TO_INCLUDE ${FILE})
      list(APPEND WEBP_SIMD_FLAGS_TO_INCLUDE ${SIMD_COMPILE_FLAG})
    endforeach()
  else()
    foreach(FILE ${SIMD_FILES})
      list(APPEND WEBP_SIMD_FILES_NOT_TO_INCLUDE ${FILE})
    endforeach()
    if(SIMD_DISABLE_FLAGS)
      list(GET SIMD_DISABLE_FLAGS ${I_SIMD} SIMD_COMPILE_FLAG)
      include(CheckCCompilerFlag)
      if(SIMD_COMPILE_FLAG)
        unset(HAS_COMPILE_FLAG)
        unset(HAS_COMPILE_FLAG CACHE)
        check_c_compiler_flag(${SIMD_COMPILE_FLAG} HAS_COMPILE_FLAG)
        if(HAS_COMPILE_FLAG)
          if(COMMAND check_compiler_flag_common_patterns)
            check_compiler_flag_common_patterns(COMMON_PATTERNS)
          else()
            set(COMMON_PATTERNS)
          endif()
          set(CMAKE_REQUIRED_DEFINITIONS ${SIMD_COMPILE_FLAG})
          check_c_source_compiles("int main(void) {return 0;}"
                                  FLAG_${SIMD_COMPILE_FLAG}
                                  FAIL_REGEX
                                  "warning: argument unused during compilation:"
                                  ${COMMON_PATTERNS})
          if(NOT FLAG_${SIMD_COMPILE_FLAG})
            unset(HAS_COMPILE_FLAG)
            unset(HAS_COMPILE_FLAG CACHE)
          endif()
        endif()
        if(HAS_COMPILE_FLAG)
          set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SIMD_COMPILE_FLAG}")
        endif()
      endif()
    endif()
  endif()
  cmake_pop_check_state()
endforeach()

if(WEBP_ENABLE_SWAP_16BIT_CSP)
  add_definitions(-DWEBP_SWAP_16BIT_CSP=1)
endif()

if(NOT WEBP_BITTRACE STREQUAL "0")
  add_definitions(-DBITTRACE=${WEBP_BITTRACE})
endif()

set(PTHREAD_LIBS ${CMAKE_THREAD_LIBS_INIT})

if(ANDROID)
  add_definitions(-DHAVE_CPU_FEATURES_H=1)
  set(HAVE_CPU_FEATURES_H 1)
  list(APPEND WEBP_DEP_LIBRARIES cpufeatures)
else()
  set(HAVE_CPU_FEATURES_H 0)
endif()

function(parse_Makefile_am FOLDER VAR SRC_REGEX)
  file(READ ${FOLDER}/Makefile.am MAKEFILE_AM)
  string(REGEX MATCHALL
               "${SRC_REGEX}_SOURCES[ ]*\\+?=[ ]+[0-9a-z\\._ ]*"
               FILES_PER_LINE
               ${MAKEFILE_AM})
  set(SRCS ${${VAR}})
  foreach(FILES ${FILES_PER_LINE})
    string(FIND ${FILES} "=" OFFSET)
    math(EXPR OFFSET "${OFFSET} + 2")
    string(SUBSTRING ${FILES}
                     ${OFFSET}
                     -1
                     FILES)
    if(FILES)
      string(REGEX MATCHALL
                   "[0-9a-z\\._]+"
                   FILES
                   ${FILES})
      foreach(FILE ${FILES})
        list(APPEND SRCS ${FOLDER}/${FILE})
      endforeach()
    endif()
  endforeach()
  set(${VAR} ${SRCS} PARENT_SCOPE)
endfunction()

set(WEBP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libwebp/src)
parse_makefile_am(${WEBP_SRC_DIR}/dec "WEBP_DEC_SRCS" "")
parse_makefile_am(${WEBP_SRC_DIR}/demux "WEBP_DEMUX_SRCS" "")
parse_makefile_am(${WEBP_SRC_DIR}/dsp "WEBP_DSP_COMMON_SRCS" "COMMON")
parse_makefile_am(${WEBP_SRC_DIR}/dsp "WEBP_DSP_ENC_SRCS" "ENC")
parse_makefile_am(${WEBP_SRC_DIR}/dsp "WEBP_DSP_ENC_SRCS" "dsp_[^ ]*")
parse_makefile_am(${WEBP_SRC_DIR}/dsp "WEBP_DSP_DEC_SRCS" "decode_[^ ]*")
parse_makefile_am(${WEBP_SRC_DIR}/enc "WEBP_ENC_SRCS" "")
parse_makefile_am(${WEBP_SRC_DIR}/utils "WEBP_UTILS_COMMON_SRCS" "COMMON")
parse_makefile_am(${WEBP_SRC_DIR}/utils "WEBP_UTILS_ENC_SRCS" "ENC")
parse_makefile_am(${WEBP_SRC_DIR}/utils "WEBP_UTILS_DEC_SRCS" "decode_[^ ]*")

foreach(FILE ${WEBP_SIMD_FILES_NOT_TO_INCLUDE})
  list(REMOVE_ITEM WEBP_DSP_ENC_SRCS ${FILE})
  list(REMOVE_ITEM WEBP_DSP_DEC_SRCS ${FILE})
endforeach()

configure_file(${CMAKE_CURRENT_LIST_DIR}/libwebp/cmake/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/src/webp/config.h)
add_definitions(-DHAVE_CONFIG_H)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
  add_definitions(-Wall)
endif()

add_library(webp ${WEBP_DEC_SRCS}
            ${WEBP_DSP_COMMON_SRCS}
            ${WEBP_DSP_DEC_SRCS}
            ${WEBP_DSP_ENC_SRCS}
            ${WEBP_ENC_SRCS}
            ${WEBP_UTILS_COMMON_SRCS}
            ${WEBP_UTILS_DEC_SRCS}
            ${WEBP_UTILS_ENC_SRCS})
target_link_libraries(webp PRIVATE ${WEBP_DEP_LIBRARIES})
target_include_directories(webp
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libwebp
          ${CMAKE_CURRENT_BINARY_DIR}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libwebp/src>
         $<INSTALL_INTERFACE:include>)

add_library(webpdemux ${WEBP_DEMUX_SRCS})
target_link_libraries(webpdemux PUBLIC webp)
target_include_directories(webpdemux
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libwebp
                                   ${CMAKE_CURRENT_BINARY_DIR}
                           PUBLIC $<INSTALL_INTERFACE:include>)

parse_makefile_am(${WEBP_SRC_DIR}/mux "WEBP_MUX_SRCS" "")
add_library(webpmux STATIC ${WEBP_MUX_SRCS})
target_link_libraries(webpmux PUBLIC webp)
target_include_directories(webpmux
                            PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
                                    ${CMAKE_CURRENT_SOURCE_DIR}/libwebp)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/libwebp/configure.ac CONFIGURE_FILE)
string(REGEX MATCH "AC_INIT\\([^\n]*\\[[0-9\\.]+\\]" TMP ${CONFIGURE_FILE})
string(REGEX MATCH "[0-9\\.]+" PROJECT_VERSION ${TMP})

list(LENGTH WEBP_SIMD_FILES_TO_INCLUDE WEBP_SIMD_FILES_TO_INCLUDE_LENGTH)
math(EXPR WEBP_SIMD_FILES_TO_INCLUDE_RANGE "${WEBP_SIMD_FILES_TO_INCLUDE_LENGTH}-1")

foreach(I_FILE RANGE ${WEBP_SIMD_FILES_TO_INCLUDE_RANGE})
  list(GET WEBP_SIMD_FILES_TO_INCLUDE ${I_FILE} FILE)
  list(GET WEBP_SIMD_FLAGS_TO_INCLUDE ${I_FILE} SIMD_COMPILE_FLAG)
  set_source_files_properties(${FILE} PROPERTIES COMPILE_FLAGS ${SIMD_COMPILE_FLAG})
endforeach()