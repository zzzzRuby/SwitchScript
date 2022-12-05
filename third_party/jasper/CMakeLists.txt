set(JAS_VERSION_MAJOR 3)
set(JAS_VERSION_MINOR 0)
set(JAS_VERSION_PATCH 5)

set(JAS_SO_VERSION 6)
set(JAS_SO_MINOR 0)
set(JAS_SO_RELEASE 0)

set(JAS_VERSION "${JAS_VERSION_MAJOR}.${JAS_VERSION_MINOR}.${JAS_VERSION_PATCH}")

project(JasPer VERSION ${JAS_VERSION} LANGUAGES C)

set(JAS_ENABLE_32BIT OFF)
set(JAS_INCLUDE_BMP_CODEC ON)
set(JAS_INCLUDE_JP2_CODEC ON)
set(JAS_INCLUDE_JPC_CODEC ON)
set(JAS_INCLUDE_JPG_CODEC ON)
set(JAS_INCLUDE_HEIC_CODEC OFF)
set(JAS_INCLUDE_MIF_CODEC ON)
set(JAS_INCLUDE_PGX_CODEC ON)
set(JAS_INCLUDE_PNM_CODEC ON)
set(JAS_INCLUDE_RAS_CODEC ON)
set(JAS_ENABLE_BMP_CODEC ON)
set(JAS_ENABLE_JP2_CODEC ON)
set(JAS_ENABLE_JPC_CODEC ON)
set(JAS_ENABLE_JPG_CODEC ON)
set(JAS_ENABLE_HEIC_CODEC OFF)
set(JAS_ENABLE_MIF_CODEC ON)
set(JAS_ENABLE_PGX_CODEC ON)
set(JAS_ENABLE_PNM_CODEC ON)
set(JAS_ENABLE_RAS_CODEC ON)

set(JAS_DEFAULT_MAX_MEM_USAGE "1073741824")
set(JAS_ENABLE_NON_THREAD_SAFE_DEBUGGING 0)
set(JAS_ENABLE_DANGEROUS_INTERNAL_TESTING_MODE 0)
set(JAS_STDC_VERSION __STDC_VERSION__)

include(${CMAKE_CURRENT_SOURCE_DIR}/jasper/build/cmake/modules/JasCheckType.cmake)

check_include_files(fcntl.h JAS_HAVE_FCNTL_H)
check_include_files(io.h JAS_HAVE_IO_H)
check_include_files(unistd.h JAS_HAVE_UNISTD_H)
check_include_files(sys/time.h JAS_HAVE_SYS_TIME_H)
check_include_files(sys/types.h JAS_HAVE_SYS_TYPES_H)

check_type_size(max_align_t MAX_ALIGN_T)
if(HAVE_MAX_ALIGN_T)
	set(JAS_HAVE_MAX_ALIGN_T 1 CACHE INTERNAL "Test JAS_HAVE_MAX_ALIGN_T")
else()
	set(JAS_HAVE_MAX_ALIGN_T 0 CACHE INTERNAL "Test JAS_HAVE_MAX_ALIGN_T")
endif()

jas_check_type("char" JAS_HAVE_CHAR SIZE JAS_SIZEOF_CHAR)
jas_check_type("int" JAS_HAVE_INT SIZE JAS_SIZEOF_INT)
jas_check_type("long" JAS_HAVE_LONG SIZE JAS_SIZEOF_LONG)
jas_check_type("long long" JAS_HAVE_LLONG SIZE JAS_SIZEOF_LLONG)
jas_check_type("size_t" JAS_HAVE_SIZE_T SIZE JAS_SIZEOF_SIZE_T)
jas_check_type("ssize_t" JAS_HAVE_SSIZE_T SIZE JAS_SIZEOF_SSIZE_T)
check_function_exists(nanosleep JAS_HAVE_NANOSLEEP)
check_function_exists(gettimeofday JAS_HAVE_GETTIMEOFDAY)
check_function_exists(getrusage JAS_HAVE_GETRUSAGE)
check_function_exists(mkostemp JAS_HAVE_MKOSTEMP)
if(MSVC)
	check_symbol_exists(snprintf stdio.h JAS_HAVE_SNPRINTF)
else()
	check_function_exists(snprintf JAS_HAVE_SNPRINTF)
endif()

set(JAS_HAVE_ALL_NATIVE_CODECS 1)
set(JAS_THREADS FALSE)
set(JAS_THREADS_C11 FALSE)
set(JAS_THREADS_PTHREAD FALSE)
set(JAS_THREADS_WIN32 FALSE)
set(JAS_HAVE_THREAD_LOCAL FALSE)

set(JAS_HAVE_OPENGL 0)
set(JAS_HAVE_LIBJPEG 0)
set(JAS_HAVE_LIBHEIF 0)
set(JAS_LIBJPEG_TARGET)
set(JAS_HAVE_LIBJPEG 1)

if(NOT JAS_HAVE_LIBHEIF)
	set(JAS_INCLUDE_HEIC_CODEC 0)
endif()

find_library(MATH_LIBRARY m)
if(NOT MATH_LIBRARY)
	set(MATH_LIBRARY "")
endif()

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/jasper/src/libjasper/include/jasper/jas_config.h.in"
	"${CMAKE_CURRENT_BINARY_DIR}/include/jasper/jas_config.h"
)

set(_libjasper_sources
	include/jasper/jas_cm.h
	include/jasper/jas_compiler.h
	include/jasper/jas_debug.h
	include/jasper/jas_dll.h
	include/jasper/jas_fix.h
	include/jasper/jas_getopt.h
	include/jasper/jas_icc.h
	include/jasper/jas_image.h
	include/jasper/jas_init.h
	include/jasper/jas_log.h
	include/jasper/jas_malloc.h
	include/jasper/jas_math.h
	include/jasper/jasper.h
	include/jasper/jas_seq.h
	include/jasper/jas_stream.h
	include/jasper/jas_string.h
	include/jasper/jas_thread.h
	include/jasper/jas_tmr.h
	include/jasper/jas_tvp.h
	include/jasper/jas_types.h
	include/jasper/jas_version.h
	base/jas_cm.c
	base/jas_debug.c
	base/jas_getopt.c
	base/jas_icc.c
	base/jas_iccdata.c
	base/jas_image.c
	base/jas_init.c
	base/jas_malloc.c
	base/jas_seq.c
	base/jas_stream.c
	base/jas_string.c
	base/jas_tmr.c
	base/jas_tvp.c
	base/jas_version.c
  bmp/bmp_cod.c
  bmp/bmp_dec.c
  bmp/bmp_enc.c
  jp2/jp2_cod.c
  jp2/jp2_dec.c
  jp2/jp2_enc.c
  jpc/jpc_bs.c
  jpc/jpc_cs.c
  jpc/jpc_cod.c
  jpc/jpc_dec.c
  jpc/jpc_enc.c
  jpc/jpc_math.c
  jpc/jpc_mct.c
  jpc/jpc_mqcod.c
  jpc/jpc_mqdec.c
  jpc/jpc_mqenc.c
  jpc/jpc_qmfb.c
  jpc/jpc_t1cod.c
  jpc/jpc_t1dec.c
  jpc/jpc_t1enc.c
  jpc/jpc_t2cod.c
  jpc/jpc_t2dec.c
  jpc/jpc_t2enc.c
  jpc/jpc_tagtree.c
  jpc/jpc_tsfb.c
  jpc/jpc_util.c
  jpg/jpg_val.c
  jpg/jpg_dec.c
  jpg/jpg_enc.c
  #heic/heic_val.c
  #heic/heic_dec.c
  #heic/heic_enc.c
  mif/mif_cod.c
  pgx/pgx_cod.c
  pgx/pgx_dec.c
  pgx/pgx_enc.c
  pnm/pnm_cod.c
  pnm/pnm_dec.c
  pnm/pnm_enc.c
  ras/ras_cod.c
  ras/ras_dec.c
  ras/ras_enc.c
)

set(libjasper_sources "${CMAKE_CURRENT_BINARY_DIR}/include/jasper/jas_config.h")
foreach(_libjasper_source ${_libjasper_sources})
    list(APPEND libjasper_sources ${CMAKE_CURRENT_SOURCE_DIR}/jasper/src/libjasper/${_libjasper_source})
endforeach()

add_library(jasper STATIC ${libjasper_sources})

set_target_properties(jasper PROPERTIES LINKER_LANGUAGE C)

target_include_directories(jasper PUBLIC
  "${CMAKE_CURRENT_BINARY_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/jasper/src/libjasper/include"
)

if(MSVC)
	target_compile_definitions(jasper PRIVATE _CRT_SECURE_NO_WARNINGS)
	target_compile_definitions(jasper PRIVATE _CRT_SECURE_NO_DEPRECATE)
	target_compile_definitions(jasper PRIVATE _CRT_NONSTDC_NO_DEPRECATE)
endif()

target_link_libraries(jasper PUBLIC ${MATH_LIBRARY} jpeg)

include(GenerateExportHeader)
generate_export_header(jasper
  EXPORT_FILE_NAME include/jasper/jas_export_cmake.h
  INCLUDE_GUARD_NAME JAS_EXPORT_CMAKE_H
  EXPORT_MACRO_NAME JAS_EXPORT
  NO_EXPORT_MACRO_NAME JAS_LOCAL
)