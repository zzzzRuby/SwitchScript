project(libjpeg-turbo C)
set(VERSION 2.1.3)
set(COPYRIGHT_YEAR "1991-2022")
string(REPLACE "." ";" VERSION_TRIPLET ${VERSION})
list(GET VERSION_TRIPLET 0 VERSION_MAJOR)
list(GET VERSION_TRIPLET 1 VERSION_MINOR)
list(GET VERSION_TRIPLET 2 VERSION_REVISION)
function(pad_number NUMBER OUTPUT_LEN)
    string(LENGTH "${${NUMBER}}" INPUT_LEN)
    if(INPUT_LEN LESS OUTPUT_LEN)
        math(EXPR ZEROES "${OUTPUT_LEN} - ${INPUT_LEN} - 1")
        set(NUM ${${NUMBER}})
        foreach(C RANGE ${ZEROES})
            set(NUM "0${NUM}")
        endforeach()
        set(${NUMBER} ${NUM} PARENT_SCOPE)
    endif()
endfunction()
pad_number(VERSION_MINOR 3)
pad_number(VERSION_REVISION 3)
set(LIBJPEG_TURBO_VERSION_NUMBER ${VERSION_MAJOR}${VERSION_MINOR}${VERSION_REVISION})

string(TIMESTAMP BUILD "%Y%m%d")

if(WIN32)
    check_symbol_exists("_M_AMD64" "" JPEG_AMD64)
    check_symbol_exists("_M_ARM64" "" JPEG_ARM64)
    check_symbol_exists("_M_IX86" "" JPEG_X86)
    check_symbol_exists("_M_ARM" "" JPEG_ARM)
else()
    check_symbol_exists("__x86_64__" "" JPEG_AMD64)
    check_symbol_exists("__aarch64__" "" JPEG_ARM64)
    check_symbol_exists("__i386__" "" JPEG_X86)
    check_symbol_exists("__arm__" "" JPEG_ARM)
endif()

if (JPEG_AMD64)
    set(CPU_TYPE x86_64)
elseif (JPEG_X86)
    set(CPU_TYPE i386)
elseif (JPEG_ARM)
    set(CPU_TYPE arm)
elseif (JPEG_ARM64)
    set(CPU_TYPE arm64)
endif()

set(BITS_IN_JSAMPLE 8)
set(D_ARITH_CODING_SUPPORTED on)
set(C_ARITH_CODING_SUPPORTED on)
set(MEM_SRCDST_SUPPORTED off)
set(JPEG_LIB_VERSION 80)

if(MSVC)
    add_definitions(-D_CRT_NONSTDC_NO_WARNINGS)
endif()

include(CheckCSourceCompiles)
include(CheckIncludeFiles)
include(CheckTypeSize)

check_type_size("size_t" SIZE_T)
check_type_size("unsigned long" UNSIGNED_LONG)

if(SIZE_T EQUAL UNSIGNED_LONG)
    check_c_source_compiles("int main(int argc, char **argv) { unsigned long a = argc;  return __builtin_ctzl(a); }"
        HAVE_BUILTIN_CTZL)
endif()
if(MSVC)
    check_include_files("intrin.h" HAVE_INTRIN_H)
endif()

if(UNIX)
    if(CMAKE_CROSSCOMPILING)
        set(RIGHT_SHIFT_IS_UNSIGNED 0)
    else()
        include(CheckCSourceRuns)
        check_c_source_runs("
        #include <stdio.h>
        #include <stdlib.h>
        int is_shifting_signed (long arg) {
            long res = arg >> 4;
            if (res == -0x7F7E80CL)
            return 1; /* right shift is signed */
            /* see if unsigned-shift hack will fix it. */
            /* we can't just test exact value since it depends on width of long... */
            res |= (~0L) << (32-4);
            if (res == -0x7F7E80CL)
            return 0; /* right shift is unsigned */
            printf(\"Right shift isn't acting as I expect it to.\\\\n\");
            printf(\"I fear the JPEG software will not work at all.\\\\n\\\\n\");
            return 0; /* try it with unsigned anyway */
        }
        int main (void) {
            exit(is_shifting_signed(-0x7F7E80B1L));
        }" RIGHT_SHIFT_IS_UNSIGNED)
    endif()
endif()

if(MSVC)
    set(THREAD_LOCAL "__declspec(thread)")
    set(INLINE "__forceinline")
else()
    set(THREAD_LOCAL "__thread")
    set(INLINE "__inline__ __attribute__((always_inline))")
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/libjpeg-turbo)

set(_JPEG_SOURCES jcapimin.c jcapistd.c jccoefct.c jccolor.c jcdctmgr.c jchuff.c
    jcicc.c jcinit.c jcmainct.c jcmarker.c jcmaster.c jcomapi.c jcparam.c
    jcphuff.c jcprepct.c jcsample.c jctrans.c jdapimin.c jdapistd.c jdatadst.c
    jdatasrc.c jdcoefct.c jdcolor.c jddctmgr.c jdhuff.c jdicc.c jdinput.c
    jdmainct.c jdmarker.c jdmaster.c jdmerge.c jdphuff.c jdpostct.c jdsample.c
    jdtrans.c jerror.c jfdctflt.c jfdctfst.c jfdctint.c jidctflt.c jidctfst.c
    jidctint.c jidctred.c jquant1.c jquant2.c jutils.c jmemmgr.c jmemnobs.c
    jaricom.c jcarith.c jdarith.c)

set(JPEG_SOURCES)
foreach(src ${_JPEG_SOURCES})
    list(APPEND JPEG_SOURCES libjpeg-turbo/${src})
endforeach()
add_library(jpeg STATIC ${JPEG_SOURCES})

if (CPU_TYPE STREQUAL "x86_64" OR CPU_TYPE STREQUAL "i386")
    enable_language(ASM_NASM)

    if(CPU_TYPE STREQUAL "x86_64")
        set(_SIMD_SOURCES x86_64/jsimdcpu.asm x86_64/jfdctflt-sse.asm
            x86_64/jccolor-sse2.asm x86_64/jcgray-sse2.asm x86_64/jchuff-sse2.asm
            x86_64/jcphuff-sse2.asm x86_64/jcsample-sse2.asm x86_64/jdcolor-sse2.asm
            x86_64/jdmerge-sse2.asm x86_64/jdsample-sse2.asm x86_64/jfdctfst-sse2.asm
            x86_64/jfdctint-sse2.asm x86_64/jidctflt-sse2.asm x86_64/jidctfst-sse2.asm
            x86_64/jidctint-sse2.asm x86_64/jidctred-sse2.asm x86_64/jquantf-sse2.asm
            x86_64/jquanti-sse2.asm
            x86_64/jccolor-avx2.asm x86_64/jcgray-avx2.asm x86_64/jcsample-avx2.asm
            x86_64/jdcolor-avx2.asm x86_64/jdmerge-avx2.asm x86_64/jdsample-avx2.asm
            x86_64/jfdctint-avx2.asm x86_64/jidctint-avx2.asm x86_64/jquanti-avx2.asm)
    else()
        set(_SIMD_SOURCES i386/jsimdcpu.asm i386/jfdctflt-3dn.asm
            i386/jidctflt-3dn.asm i386/jquant-3dn.asm
            i386/jccolor-mmx.asm i386/jcgray-mmx.asm i386/jcsample-mmx.asm
            i386/jdcolor-mmx.asm i386/jdmerge-mmx.asm i386/jdsample-mmx.asm
            i386/jfdctfst-mmx.asm i386/jfdctint-mmx.asm i386/jidctfst-mmx.asm
            i386/jidctint-mmx.asm i386/jidctred-mmx.asm i386/jquant-mmx.asm
            i386/jfdctflt-sse.asm i386/jidctflt-sse.asm i386/jquant-sse.asm
            i386/jccolor-sse2.asm i386/jcgray-sse2.asm i386/jchuff-sse2.asm
            i386/jcphuff-sse2.asm i386/jcsample-sse2.asm i386/jdcolor-sse2.asm
            i386/jdmerge-sse2.asm i386/jdsample-sse2.asm i386/jfdctfst-sse2.asm
            i386/jfdctint-sse2.asm i386/jidctflt-sse2.asm i386/jidctfst-sse2.asm
            i386/jidctint-sse2.asm i386/jidctred-sse2.asm i386/jquantf-sse2.asm
            i386/jquanti-sse2.asm
            i386/jccolor-avx2.asm i386/jcgray-avx2.asm i386/jcsample-avx2.asm
            i386/jdcolor-avx2.asm i386/jdmerge-avx2.asm i386/jdsample-avx2.asm
            i386/jfdctint-avx2.asm i386/jidctint-avx2.asm i386/jquanti-avx2.asm)
    endif()

    set(SIMD_SOURCES)
    foreach(src ${_SIMD_SOURCES})
        list(APPEND SIMD_SOURCES libjpeg-turbo/simd/${src})
    endforeach()
    set_source_files_properties(${SIMD_SOURCES} PROPERTIES LANGUAGE ASM_NASM)
    if (CPU_TYPE STREQUAL "x86_64")
        if ("${CMAKE_ASM_NASM_OBJECT_FORMAT}" MATCHES "^win")
            set_source_files_properties(${SIMD_SOURCES} PROPERTIES
                COMPILE_DEFINITIONS "__x86_64__;WIN64"
                COMPILE_FLAGS -UWIN32)
        elseif ("${CMAKE_ASM_NASM_OBJECT_FORMAT}" MATCHES "^elf")
            set_source_files_properties(${SIMD_SOURCES} PROPERTIES
                COMPILE_DEFINITIONS "__x86_64__;ELF")
        elseif ("${CMAKE_ASM_NASM_OBJECT_FORMAT}" MATCHES "^macho")
            set_source_files_properties(${SIMD_SOURCES} PROPERTIES
                COMPILE_DEFINITIONS "__x86_64__;MACHO")
        endif()
    else()
        if ("${CMAKE_ASM_NASM_OBJECT_FORMAT}" MATCHES "^win")
            set_source_files_properties(${SIMD_SOURCES} PROPERTIES
                COMPILE_DEFINITIONS "WIN32"
                COMPILE_FLAGS -UWIN32)
        elseif ("${CMAKE_ASM_NASM_OBJECT_FORMAT}" MATCHES "^elf")
            set_source_files_properties(${SIMD_SOURCES} PROPERTIES
                COMPILE_DEFINITIONS "ELF")
        elseif ("${CMAKE_ASM_NASM_OBJECT_FORMAT}" MATCHES "^macho")
            set_source_files_properties(${SIMD_SOURCES} PROPERTIES
                COMPILE_DEFINITIONS "MACHO")
        endif()
    endif()
    target_include_directories(jpeg PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/libjpeg-turbo/simd/nasm
        ${CMAKE_CURRENT_SOURCE_DIR}/libjpeg-turbo/simd/${CPU_TYPE})
    target_sources(jpeg PRIVATE ${SIMD_SOURCES} libjpeg-turbo/simd/${CPU_TYPE}/jsimd.c)

    set(WITH_SIMD on)
elseif(CPU_TYPE STREQUAL "arm64" OR CPU_TYPE STREQUAL "arm")
    if(CPU_TYPE STREQUAL "arm")
        check_c_source_compiles("
            #if defined(__ARM_NEON__) || (!defined(__linux__) && !defined(ANDROID) && !defined(__ANDROID__))
            #error \"Neon run-time auto-detection will not be used\"
            #endif
            #if __ARM_PCS_VFP == 1
            #error \"float ABI = hard\"
            #endif
            #if __SOFTFP__ != 1
            #error \"float ABI = softfp\"
            #endif
            int main(void) { return 0; }" NEED_SOFTFP_FOR_INTRINSICS)
        if(NEED_SOFTFP_FOR_INTRINSICS)
            set(SOFTFP_FLAG -mfloat-abi=softfp)
        endif()
    endif()

    if(CPU_TYPE STREQUAL "arm")
        set(CMAKE_REQUIRED_FLAGS "-mfpu=neon ${SOFTFP_FLAG}")
        check_c_source_compiles("
            #include <arm_neon.h>
            int main(int argc, char **argv) {
            uint16x8_t input = vdupq_n_u16((uint16_t)argc);
            uint8x8_t output = vmovn_u16(input);
            return (int)output[0];
            }" HAVE_NEON)
        if(NOT HAVE_NEON)
            message(FATAL_ERROR "SIMD extensions not available for this architecture")
            return()
        endif()
    endif()
    check_c_source_compiles("
    #include <arm_neon.h>
    int main(int argc, char **argv) {
        int16_t input[] = {
        (int16_t)argc, (int16_t)argc, (int16_t)argc, (int16_t)argc,
        (int16_t)argc, (int16_t)argc, (int16_t)argc, (int16_t)argc,
        (int16_t)argc, (int16_t)argc, (int16_t)argc, (int16_t)argc
        };
        int16x4x3_t output = vld1_s16_x3(input);
        vst3_s16(input, output);
        return (int)input[0];
    }" HAVE_VLD1_S16_X3)
    check_c_source_compiles("
    #include <arm_neon.h>
    int main(int argc, char **argv) {
        uint16_t input[] = {
        (uint16_t)argc, (uint16_t)argc, (uint16_t)argc, (uint16_t)argc,
        (uint16_t)argc, (uint16_t)argc, (uint16_t)argc, (uint16_t)argc
        };
        uint16x4x2_t output = vld1_u16_x2(input);
        vst2_u16(input, output);
        return (int)input[0];
    }" HAVE_VLD1_U16_X2)
    check_c_source_compiles("
    #include <arm_neon.h>
    int main(int argc, char **argv) {
        uint8_t input[] = {
        (uint8_t)argc, (uint8_t)argc, (uint8_t)argc, (uint8_t)argc,
        (uint8_t)argc, (uint8_t)argc, (uint8_t)argc, (uint8_t)argc,
        (uint8_t)argc, (uint8_t)argc, (uint8_t)argc, (uint8_t)argc,
        (uint8_t)argc, (uint8_t)argc, (uint8_t)argc, (uint8_t)argc,
        (uint8_t)argc, (uint8_t)argc, (uint8_t)argc, (uint8_t)argc,
        (uint8_t)argc, (uint8_t)argc, (uint8_t)argc, (uint8_t)argc,
        (uint8_t)argc, (uint8_t)argc, (uint8_t)argc, (uint8_t)argc,
        (uint8_t)argc, (uint8_t)argc, (uint8_t)argc, (uint8_t)argc,
        (uint8_t)argc, (uint8_t)argc, (uint8_t)argc, (uint8_t)argc,
        (uint8_t)argc, (uint8_t)argc, (uint8_t)argc, (uint8_t)argc,
        (uint8_t)argc, (uint8_t)argc, (uint8_t)argc, (uint8_t)argc,
        (uint8_t)argc, (uint8_t)argc, (uint8_t)argc, (uint8_t)argc,
        (uint8_t)argc, (uint8_t)argc, (uint8_t)argc, (uint8_t)argc,
        (uint8_t)argc, (uint8_t)argc, (uint8_t)argc, (uint8_t)argc,
        (uint8_t)argc, (uint8_t)argc, (uint8_t)argc, (uint8_t)argc,
        (uint8_t)argc, (uint8_t)argc, (uint8_t)argc, (uint8_t)argc
        };
        uint8x16x4_t output = vld1q_u8_x4(input);
        vst4q_u8(input, output);
        return (int)input[0];
    }" HAVE_VLD1Q_U8_X4)
    configure_file(libjpeg-turbo/simd/arm/neon-compat.h.in arm/neon-compat.h @ONLY)
    include_directories(${CMAKE_CURRENT_BINARY_DIR}/arm)

    add_definitions(-DNEON_INTRINSICS)

    set(SIMD_SOURCES arm/jcgray-neon.c arm/jcphuff-neon.c arm/jcsample-neon.c
        arm/jdmerge-neon.c arm/jdsample-neon.c arm/jfdctfst-neon.c
        arm/jidctred-neon.c arm/jquanti-neon.c arm/jccolor-neon.c arm/jidctint-neon.c
        arm/jidctfst-neon.c arm/jdcolor-neon.c arm/jfdctint-neon.c)
    if (CPU_TYPE STREQUAL "arm")
        list(APPEND SIMD_SOURCES arm/aarch32/jchuff-neon.c)
    else()
        list(APPEND SIMD_SOURCES arm/aarch64/jchuff-neon.c)
    endif()

    set(_SIMD_SOURCES)
    foreach(src ${SIMD_SOURCES})
        list(APPEND _SIMD_SOURCES libjpeg-turbo/simd/${src})
    endforeach()

    if(CPU_TYPE STREQUAL "arm")
        set_source_files_properties(${_SIMD_SOURCES} COMPILE_FLAGS "-mfpu=neon ${SOFTFP_FLAG}")
    endif()

    if (CPU_TYPE STREQUAL "arm")
        target_sources(jpeg PRIVATE ${_SIMD_SOURCES} libjpeg-turbo/simd/arm/aarch32/jsimd.c)
    else()
        target_sources(jpeg PRIVATE ${_SIMD_SOURCES} libjpeg-turbo/simd/arm/aarch64/jsimd.c)
    endif()
    set(WITH_SIMD on)
else()
    target_sources(jpeg PRIVATE libjpeg-turbo/jsimd_none.c)
    set(WITH_SIMD off)
endif()
target_include_directories(jpeg INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libjpeg-turbo ${CMAKE_CURRENT_BINARY_DIR})

add_library(turbojpeg STATIC
    libjpeg-turbo/turbojpeg.c
    libjpeg-turbo/transupp.c
    libjpeg-turbo/jdatadst-tj.c
    libjpeg-turbo/jdatasrc-tj.c
    libjpeg-turbo/rdbmp.c
    libjpeg-turbo/rdppm.c
    libjpeg-turbo/wrbmp.c
    libjpeg-turbo/wrppm.c)
target_link_libraries(turbojpeg PRIVATE jpeg)
target_compile_definitions(turbojpeg PRIVATE BMP_SUPPORTED PPM_SUPPORTED)

if(WIN32)
    configure_file(libjpeg-turbo/win/jconfig.h.in jconfig.h)
else()
    configure_file(libjpeg-turbo/jconfig.h.in jconfig.h)
endif()
configure_file(libjpeg-turbo/jconfigint.h.in jconfigint.h)
configure_file(libjpeg-turbo/jversion.h.in jversion.h)
