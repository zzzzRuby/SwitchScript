set(ASM686 off)
set(AMD64 off)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCSourceCompiles)

check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(stdint.h    HAVE_STDINT_H)
check_include_file(stddef.h    HAVE_STDDEF_H)

set(CMAKE_REQUIRED_DEFINITIONS -D_LARGEFILE64_SOURCE=1)
if(HAVE_SYS_TYPES_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_SYS_TYPES_H)
endif()
if(HAVE_STDINT_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDINT_H)
endif()
if(HAVE_STDDEF_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDDEF_H)
endif()
check_type_size(off64_t OFF64_T)
if(HAVE_OFF64_T)
   add_definitions(-D_LARGEFILE64_SOURCE=1)
endif()
set(CMAKE_REQUIRED_DEFINITIONS) # clear variable

check_function_exists(fseeko HAVE_FSEEKO)
if(NOT HAVE_FSEEKO)
    add_definitions(-DNO_FSEEKO)
endif()

check_include_file(unistd.h Z_HAVE_UNISTD_H)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/zlib/zconf.h.cmakein
        ${CMAKE_CURRENT_BINARY_DIR}/zconf.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(ZLIB_SRCS
    zlib.h
    crc32.h
    deflate.h
    gzguts.h
    inffast.h
    inffixed.h
    inflate.h
    inftrees.h
    trees.h
    zutil.h
    adler32.c
    compress.c
    crc32.c
    deflate.c
    gzclose.c
    gzlib.c
    gzread.c
    gzwrite.c
    inflate.c
    infback.c
    inftrees.c
    inffast.c
    trees.c
    uncompr.c
    zutil.c
)

if(CMAKE_COMPILER_IS_GNUCC)
    if(ASM686)
        set(ZLIB_ASMS contrib/asm686/match.S)
    elseif (AMD64)
        set(ZLIB_ASMS contrib/amd64/amd64-match.S)
    endif ()

    if(ZLIB_ASMS)
        add_definitions(-DASMV)
            foreach(asm_src ${ZLIB_ASMS})
                set_source_files_properties(
                   ${CMAKE_CURRENT_SOURCE_DIR}/zlib/${asm_src}
                    PROPERTIES COMPILE_FLAGS -DNO_UNDERLINE)
            endforeach()
    endif()
endif()

if(MSVC)
    if(ASM686)
        enable_language(ASM_MASM)
        set(ZLIB_ASMS
            contrib/masmx86/inffas32.asm
            contrib/masmx86/match686.asm
        )
        foreach(asm_src ${ZLIB_ASMS})
            set_source_files_properties(
                ${CMAKE_CURRENT_SOURCE_DIR}/zlib/${asm_src}
                PROPERTIES LANGUAGE ASM_MASM)
        endforeach()
    elseif (AMD64)
        enable_language(ASM_MASM)
        set(ZLIB_ASMS
            contrib/masmx64/gvmat64.asm
            contrib/masmx64/inffasx64.asm
        )
        foreach(asm_src ${ZLIB_ASMS})
            set_source_files_properties(
                ${CMAKE_CURRENT_SOURCE_DIR}/zlib/${asm_src}
                PROPERTIES LANGUAGE ASM_MASM)
        endforeach()
        list(APPEND ZLIB_SRCS contrib/masmx64/inffas8664.c)
    endif()

    if(ZLIB_ASMS)
        add_definitions(-DASMV -DASMINF)
    endif()
endif()

set(temp)
foreach(src ${ZLIB_ASMS})
    list(APPEND temp ${CMAKE_CURRENT_SOURCE_DIR}/zlib/${src})
endforeach()
foreach(src ${ZLIB_SRCS})
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/zlib/${src} ${CMAKE_CURRENT_BINARY_DIR}/${src})
    list(APPEND temp ${CMAKE_CURRENT_BINARY_DIR}/${src})
endforeach()

add_library(zlib STATIC ${temp})
target_include_directories(zlib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> $<INSTALL_INTERFACE:include>)
set_target_properties(zlib PROPERTIES OUTPUT_NAME z_static)
target_sources(zlib PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_BINARY_DIR}
    FILES ${CMAKE_CURRENT_BINARY_DIR}/zconf.h ${CMAKE_CURRENT_BINARY_DIR}/zlib.h)