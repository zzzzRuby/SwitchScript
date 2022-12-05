include_directories(AFTER
  ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src
  ${CMAKE_CURRENT_BINARY_DIR}/src
)

include (FindPythonInterp)

include (CheckFunctionExists)
include (CheckIncludeFile)
macro (check_funcs)
  foreach (func_name ${ARGN})
    string(TOUPPER ${func_name} definition_to_add)
    check_function_exists(${func_name} HAVE_${definition_to_add})
    if (${HAVE_${definition_to_add}})
      add_definitions(-DHAVE_${definition_to_add})
    endif ()
  endforeach ()
endmacro ()
if (UNIX)
  list(APPEND CMAKE_REQUIRED_LIBRARIES m)
endif ()
check_funcs(atexit mprotect sysconf getpagesize mmap isatty)
check_include_file(unistd.h HAVE_UNISTD_H)
if (${HAVE_UNISTD_H})
  add_definitions(-DHAVE_UNISTD_H)
endif ()
check_include_file(sys/mman.h HAVE_SYS_MMAN_H)
if (${HAVE_SYS_MMAN_H})
  add_definitions(-DHAVE_SYS_MMAN_H)
endif ()
check_include_file(stdbool.h HAVE_STDBOOL_H)
if (${HAVE_STDBOOL_H})
  add_definitions(-DHAVE_STDBOOL_H)
endif ()

if (MSVC)
  add_definitions(-wd4244 -wd4267 -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
endif ()

## Extract variables from Makefile files
function (extract_make_variable variable makefile_source)
  string(REGEX MATCH "${variable} = ([^$]+)\\$" temp "${makefile_source}")
  string(REGEX MATCHALL "[^ \n\t\\]+" listVar "${CMAKE_MATCH_1}")
  set (${variable} ${listVar} PARENT_SCOPE)
endfunction ()

function (add_prefix_to_list var prefix)
  set (listVar "")
  foreach (f ${${var}})
    list(APPEND listVar "${prefix}${f}")
  endforeach ()
  set (${var} "${listVar}" PARENT_SCOPE)
endfunction ()

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/Makefile.sources SRCSOURCES)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/util/Makefile.sources UTILSOURCES)

extract_make_variable(HB_BASE_headers ${SRCSOURCES})
add_prefix_to_list(HB_BASE_headers "${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/")

extract_make_variable(HB_SUBSET_sources ${SRCSOURCES})
add_prefix_to_list(HB_SUBSET_sources "${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/")

extract_make_variable(HB_SUBSET_headers ${SRCSOURCES})
add_prefix_to_list(HB_SUBSET_headers "${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/")
extract_make_variable(HB_BASE_RAGEL_GENERATED_sources ${SRCSOURCES})
extract_make_variable(HB_VIEW_sources ${UTILSOURCES})
add_prefix_to_list(HB_VIEW_sources "${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/util/")
extract_make_variable(HB_SHAPE_sources ${UTILSOURCES})
add_prefix_to_list(HB_SHAPE_sources "${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/util/")
extract_make_variable(HB_SUBSET_CLI_sources ${UTILSOURCES})
add_prefix_to_list(HB_SUBSET_CLI_sources "${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/util/")
extract_make_variable(HB_OT_SHAPE_CLOSURE_sources ${UTILSOURCES})
add_prefix_to_list(HB_OT_SHAPE_CLOSURE_sources "${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/util/")


file(READ ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/configure.ac CONFIGUREAC)
string(REGEX MATCH "\\[(([0-9]+)\\.([0-9]+)\\.([0-9]+))\\]" HB_VERSION_MATCH ${CONFIGUREAC})
set (HB_VERSION ${CMAKE_MATCH_1})
set (HB_VERSION_MAJOR ${CMAKE_MATCH_2})
set (HB_VERSION_MINOR ${CMAKE_MATCH_3})
set (HB_VERSION_MICRO ${CMAKE_MATCH_4})

## Define sources and headers of the project
set (project_sources ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/harfbuzz.cc) # use amalgam source
set (subset_project_sources ${HB_SUBSET_sources})
set (project_extra_sources)
set (project_headers ${HB_BASE_headers})
set (subset_project_headers ${HB_SUBSET_headers})

list(APPEND THIRD_PARTY_LIBS freetype)
add_definitions(-DHAVE_FREETYPE=1)
list(APPEND project_headers ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/hb-ft.h)
set(HAVE_FT_GET_VAR_BLEND_COORDINATES on)
set(HAVE_FT_SET_VAR_BLEND_COORDINATES on)
set(HAVE_FT_DONE_MM_VAR on)

## Atomic ops availability detection
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/try_compile_intel_atomic_primitives.c"
"        void memory_barrier (void) { __sync_synchronize (); }
        int atomic_add (int *i) { return __sync_fetch_and_add (i, 1); }
        int mutex_trylock (int *m) { return __sync_lock_test_and_set (m, 1); }
        void mutex_unlock (int *m) { __sync_lock_release (m); }
        int main () { return 0; }
")
try_compile(HB_HAVE_INTEL_ATOMIC_PRIMITIVES
  ${CMAKE_CURRENT_BINARY_DIR}/try_compile_intel_atomic_primitives
  ${CMAKE_CURRENT_BINARY_DIR}/try_compile_intel_atomic_primitives.c)
if (HB_HAVE_INTEL_ATOMIC_PRIMITIVES)
  add_definitions(-DHAVE_INTEL_ATOMIC_PRIMITIVES)
endif ()

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/try_compile_solaris_atomic_ops.c"
"        #include <atomic.h>
        /* This requires Solaris Studio 12.2 or newer: */
        #include <mbarrier.h>
        void memory_barrier (void) { __machine_rw_barrier (); }
        int atomic_add (volatile unsigned *i) { return atomic_add_int_nv (i, 1); }
        void *atomic_ptr_cmpxchg (volatile void **target, void *cmp, void *newval) { return atomic_cas_ptr (target, cmp, newval); }
        int main () { return 0; }
")
try_compile(HB_HAVE_SOLARIS_ATOMIC_OPS
  ${CMAKE_CURRENT_BINARY_DIR}/try_compile_solaris_atomic_ops
  ${CMAKE_CURRENT_BINARY_DIR}/try_compile_solaris_atomic_ops.c)
if (HB_HAVE_SOLARIS_ATOMIC_OPS)
  add_definitions(-DHAVE_SOLARIS_ATOMIC_OPS)
endif ()

add_library(harfbuzz ${project_sources} ${project_extra_sources} ${project_headers})
if (MSVC)
  target_compile_options(harfbuzz PRIVATE /utf8)
endif()
target_link_libraries(harfbuzz PRIVATE ${THIRD_PARTY_LIBS})
target_include_directories(harfbuzz PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src> $<INSTALL_INTERFACE:include>)
set_target_properties(harfbuzz PROPERTIES CXX_STANDARD 14)
target_sources(harfbuzz PUBLIC
  FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/hb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/hb-blob.h
    ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/hb-buffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/hb-common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/hb-deprecated.h
    ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/hb-draw.h
    ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/hb-face.h
    ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/hb-font.h
    ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/hb-map.h
    ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/hb-set.h
    ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/hb-shape.h
    ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/hb-shape-plan.h
    ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/hb-style.h
    ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/hb-unicode.h
    ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz/src/hb-version.h)