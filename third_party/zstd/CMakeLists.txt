set(zstd_dir ${CMAKE_CURRENT_SOURCE_DIR}/zstd/lib)
set(zstd_src )
set(zstd_definitions )

file(STRINGS "${zstd_dir}/zstd.h" ZSTD_VERSION_INFO)
foreach(line ${ZSTD_VERSION_INFO})
    if(line MATCHES "define[\t ]+(ZSTD_VERSION_MAJOR|ZSTD_VERSION_MINOR|ZSTD_VERSION_RELEASE)[\t ]+([0-9]+)")
        set(${CMAKE_MATCH_1} ${CMAKE_MATCH_2})
    endif()
endforeach()
set(VERSION "${ZSTD_VERSION_MAJOR}.${ZSTD_VERSION_MINOR}.${ZSTD_VERSION_RELEASE}")

if(WIN32)
    check_symbol_exists("_M_AMD64" "" ZSTD_IS_X64)
else()
    check_symbol_exists("__x86_64__" "" ZSTD_IS_X64)
endif()

file(GLOB zstd_compress_src CONFIGURE_DEPENDS ${zstd_dir}/compress/*.c ${zstd_dir}/compress/*.h)
source_group(compress FILES ${zstd_compress_src})
list(APPEND zstd_src ${zstd_compress_src})

file(GLOB zstd_decompress_src CONFIGURE_DEPENDS ${zstd_dir}/decompress/*.c ${zstd_dir}/decompress/*.h)
if (NOT WIN32 AND ZSTD_IS_X64)
    list(APPEND zstd_decompress_src ${zstd_dir}/decompress/huf_decompress_amd64.S)
endif()
source_group(decompress FILES ${zstd_decompress_src})
list(APPEND zstd_src ${zstd_decompress_src})

file(GLOB zstd_deprecated_src CONFIGURE_DEPENDS ${zstd_dir}/deprecated/*.c ${zstd_dir}/deprecated/*.h)
source_group(deprecated FILES ${zstd_deprecated_src})
list(APPEND zstd_src ${zstd_deprecated_src})

file(GLOB zstd_legacy_src CONFIGURE_DEPENDS ${zstd_dir}/legacy/*.c ${zstd_dir}/legacy/*.h)
source_group(legacy FILES ${zstd_legacy_src})
list(APPEND zstd_definitions ZSTD_LEGACY_SUPPORT=4)
list(APPEND zstd_src ${zstd_legacy_src})

file(GLOB zstd_zdict_src CONFIGURE_DEPENDS ${zstd_dir}/dictBuilder/*.c ${zstd_dir}/dictBuilder/*.h)
source_group(zdict FILES ${zstd_zdict_src})
list(APPEND zstd_src ${zstd_zdict_src})

file(GLOB zstd_common_src CONFIGURE_DEPENDS ${zstd_dir}/common/*.c ${zstd_dir}/common/*.h)
source_group(common FILES ${zstd_common_src})
list(APPEND zstd_definitions ZSTD_MULTITHREAD XXH_NAMESPACE=ZSTD_)
list(APPEND zstd_src ${zstd_common_src})

add_library(zstd STATIC ${zstd_src})
target_include_directories(zstd PUBLIC $<BUILD_INTERFACE:${zstd_dir}>)
target_include_directories(zstd PRIVATE ${zstd_dir}/common ${zstd_dir}/legacy)
target_compile_definitions(zstd PRIVATE ${zstd_definitions} ZSTD_TRACE=0)
if (NOT MSVC)
    target_compile_definitions(zstd PUBLIC ZSTDERRORLIB_VISIBILITY= ZSTDLIB_VISIBLE= ZSTDLIB_HIDDEN=)
endif()
find_package(Threads)
if (TARGET Threads::Threads)
    target_link_libraries(zstd PRIVATE Threads::Threads)
endif()
set_target_properties(zstd PROPERTIES VERSION ${VERSION})

target_sources(zstd PUBLIC
  FILE_SET HEADERS
  BASE_DIRS ${zstd_dir}
  FILES ${zstd_dir}/zstd.h ${zstd_dir}/zstd_errors.h ${zstd_dir}/zdict.h)

add_library(zstd::libzstd_static ALIAS zstd)
