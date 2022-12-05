cmake_minimum_required(VERSION 3.23)

project(QtWrapper)

option(Qt_BUILD_STATIC "build static qt" off)
set(QT_SILENCE_CMAKE_GENERATOR_WARNING on)
if (Qt_BUILD_STATIC)
    set(BUILD_SHARED_LIBS off CACHE INTERNAL "")
else()
    set(BUILD_SHARED_LIBS on CACHE INTERNAL "")
endif()

set(INPUT_sql no)
set(INPUT_opengl no)
set(INPUT_png system)
set(INPUT_harfbuzz system)
set(INPUT_freetype system)
set(INPUT_libjpeg system)
set(INPUT_zlib system)
if (APPLE)
    set(INPUT_framework no)
endif()
if (Qt_BUILD_STATIC)
    if (WIN32)
        if (NOT "${CMAKE_MSVC_RUNTIME_LIBRARY}" MATCHES "DLL")
            set(INPUT_static_runtime yes)
        else()
            set(INPUT_static_runtime no)
        endif()
    endif()
endif()

if ("${CMAKE_GENERATOR}" MATCHES "Visual Studio")
    add_compile_options("$<$<CONFIG:Debug>:/Ob0>" "$<$<CONFIG:Debug>:/Od>")
endif()

add_library(WrapSystemHarfbuzz::WrapSystemHarfbuzz INTERFACE IMPORTED GLOBAL)
target_link_libraries(WrapSystemHarfbuzz::WrapSystemHarfbuzz INTERFACE harfbuzz)

add_library(WrapSystemZLIB::WrapSystemZLIB INTERFACE IMPORTED GLOBAL)
target_link_libraries(WrapSystemZLIB::WrapSystemZLIB INTERFACE zlib)

add_library(WrapSystemFreetype::WrapSystemFreetype INTERFACE IMPORTED GLOBAL)
target_link_libraries(WrapSystemFreetype::WrapSystemFreetype INTERFACE freetype)

add_library(WrapSystemPNG::WrapSystemPNG INTERFACE IMPORTED GLOBAL)
target_link_libraries(WrapSystemPNG::WrapSystemPNG INTERFACE png)

add_library(WrapDoubleConversion::WrapDoubleConversion INTERFACE IMPORTED GLOBAL)
target_link_libraries(WrapDoubleConversion::WrapDoubleConversion INTERFACE double-conversion)

add_library(WrapSystemJpeg::WrapSystemJpeg INTERFACE IMPORTED GLOBAL)
target_link_libraries(WrapSystemJpeg::WrapSystemJpeg INTERFACE jpeg)
set(JPEG_FOUND on)

add_library(WrapZSTD::WrapZSTD INTERFACE IMPORTED GLOBAL)
target_link_libraries(WrapZSTD::WrapZSTD INTERFACE zstd)
get_target_property(zstd_VERSION zstd VERSION)

add_library(WrapWebP::WrapWebP INTERFACE IMPORTED GLOBAL)
target_link_libraries(WrapWebP::WrapWebP INTERFACE webp webpmux webpdemux)

add_library(WrapJasper::WrapJasper INTERFACE IMPORTED GLOBAL)
target_link_libraries(WrapJasper::WrapJasper INTERFACE jasper)

set(cache_flags 
    CMAKE_C_FLAGS CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_DEBUG
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_DEBUG
    CMAKE_ASM_FLAGS CMAKE_ASM_FLAGS_RELEASE CMAKE_ASM_FLAGS_DEBUG
    CMAKE_SHARED_LINKER_FLAGS CMAKE_SHARED_LINKER_FLAGS_RELEASE CMAKE_SHARED_LINKER_FLAGS_DEBUG
    CMAKE_EXE_LINKER_FLAGS CMAKE_EXE_LINKER_FLAGS_RELEASE CMAKE_EXE_LINKER_FLAGS_DEBUG
    CMAKE_STATIC_LINKER_FLAGS CMAKE_STATIC_LINKER_FLAGS_RELEASE CMAKE_STATIC_LINKER_FLAGS_DEBUG
    CMAKE_MODULE_LINKER_FLAGS CMAKE_MODULE_LINKER_FLAGS_RELEASE CMAKE_MODULE_LINKER_FLAGS_DEBUG)

set(CMAKE_MODULE_PATH_OLD ${CMAKE_MODULE_PATH})
foreach(flag ${cache_flags})
    set(CACHED_${flag}_OLD "${${flag}}")
endforeach()

add_subdirectory(qtmodules)

if (TARGET QtLibraryInfo)
    get_directory_property(compile_options COMPILE_OPTIONS)
    target_compile_options(QtLibraryInfo BEFORE PRIVATE ${compile_options})
endif()

if (NOT "${CMAKE_GENERATOR}" MATCHES "Ninja")
    macro(fix_exclude_all target)
        if (TARGET ${target})
            set_target_properties(${target} PROPERTIES EXCLUDE_FROM_ALL on)
        endif()
    endmacro()

    fix_exclude_all(windeployqt)
    fix_exclude_all(macdeployqt)
    fix_exclude_all(androiddeployqt)
    fix_exclude_all(uic)
    fix_exclude_all(moc)
    fix_exclude_all(rcc)
    fix_exclude_all(qmake)
    fix_exclude_all(cmake_automoc_parser)
    fix_exclude_all(qvkgen)
    fix_exclude_all(qtpaths)
    fix_exclude_all(qsb)
    fix_exclude_all(qlalr)
    fix_exclude_all(tracegen)
    fix_exclude_all(androidtestrunner)
    fix_exclude_all(qdbusxml2cpp)
    fix_exclude_all(qdbuscpp2xml)

    if (TARGET Bootstrap)
        set_target_properties(Bootstrap PROPERTIES EXCLUDE_FROM_ALL on)
    endif()
endif()

if (NOT WIN32 AND NOT APPLE)
    execute_process(
        COMMAND git rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/linuxdeployqt
        OUTPUT_VARIABLE GIT_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND git rev-list --tags --skip=1 --max-count=1
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/linuxdeployqt
        OUTPUT_VARIABLE GIT_TAG_ID
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND git describe --tags ${GIT_TAG_ID} --abbrev=0
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/linuxdeployqt
        OUTPUT_VARIABLE GIT_TAG_NAME
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(BUILD_NUMBER "<dev>")
    string(TIMESTAMP BUILD_NUMBER "%Y%m%d")
    
    add_executable(linuxdeployqt
            linuxdeployqt/tools/linuxdeployqt/main.cpp
            linuxdeployqt/tools/linuxdeployqt/shared.cpp)
    target_link_libraries(linuxdeployqt PRIVATE Qt6::Core)
    target_compile_definitions(linuxdeployqt PRIVATE
        "LINUXDEPLOYQT_VERSION=\"${GIT_TAG_NAME}\""
        "LINUXDEPLOYQT_GIT_COMMIT=\"${GIT_COMMIT}\""
        "BUILD_DATE=\"${DATE}\""
        "BUILD_NUMBER=\"${BUILD_NUMBER}\"")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH_OLD})
foreach(flag ${cache_flags})
    get_property(help_text CACHE ${flag} PROPERTY HELPSTRING)
    set(${flag} "${CACHED_${flag}_OLD}" CACHE STRING "${help_text}" FORCE)
endforeach()

unset(BUILD_SHARED_LIBS CACHE)
set(BUILD_SHARED_LIBS off)

function(qt_deploy target)
    set(target_list ${target})
    set(processed_list )
    while(target_list)
        list(GET target_list 0 next_target)
        list(REMOVE_AT target_list 0)
        if (${next_target} IN_LIST processed_list)
            continue()
        endif()
        list(APPEND processed_list ${next_target})
        if (NOT TARGET ${next_target})
            continue()
        endif()
        get_target_property(link_libraries ${next_target} LINK_LIBRARIES)

        if (NOT link_libraries)
            continue()
        endif()
        list(REMOVE_ITEM link_libraries ${processed_list})
        list(APPEND target_list ${link_libraries})

        get_target_property(target_plugins ${next_target} QT_PLUGINS)
        if (NOT target_plugins)
            continue()
        endif()

        if (NOT Qt_BUILD_STATIC)
            set_target_properties(${target_plugins} PROPERTIES EXCLUDE_FROM_ALL $<TARGET_PROPERTY:${next_target},EXCLUDE_FROM_ALL>)
            add_dependencies(${target} ${target_plugins})
        else()
            foreach(plugin ${target_plugins})
                get_target_property(default_plugin ${plugin} QT_DEFAULT_PLUGIN)
                if (default_plugin)
                    target_link_libraries(${target} PRIVATE ${plugin}_init ${plugin})
                endif()
            endforeach()
        endif()
    endwhile()

    if (NOT Qt_BUILD_STATIC)
        if (WIN32)
            add_dependencies(${target} qtpaths windeployqt)
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND Qt6::windeployqt
                    $<TARGET_FILE:${target}>
                    --no-compiler-runtime
                    $<$<CONFIG:Debug>:--debug>
                    $<$<CONFIG:Release>:--release>)
        endif()
    endif()
endfunction()

function(qt_install target)
    if (NOT Qt_BUILD_STATIC)
        cmake_parse_arguments("args" "" "DESTINATION" "CONFIGURATIONS" ${ARGN})
        if (args_CONFIGURATIONS)
            string(TOUPPER "${args_CONFIGURATIONS}" configurations)
        else()
            set(configurations "RELEASE;DEBUG")
        endif()
        if (APPLE)
            add_dependencies(${target} qtpaths macdeployqt)
            _install(CODE "
                string(TOUPPER \"\${CMAKE_INSTALL_CONFIG_NAME}\" install_config)
                if (\"\${install_config}\" MATCHES \"${configurations}\")
                    get_filename_component(tmp \"$<TARGET_BUNDLE_DIR:${target}>\" NAME)
                    execute_process(COMMAND \"$<TARGET_FILE:macdeployqt>\"
                        \"\${CMAKE_INSTALL_PREFIX}/${args_DESTINATION}/\${tmp}\"
                        $<$<CONFIG:Debug>:-use-debug-libs>)
                endif()
            ")
        elseif (WIN32)
            add_dependencies(${target} qtpaths windeployqt)
            _install(CODE "
                string(TOUPPER \"\${CMAKE_INSTALL_CONFIG_NAME}\" install_config)
                if (\"\${install_config}\" MATCHES \"${configurations}\")
                    execute_process(COMMAND \"$<TARGET_FILE:windeployqt>\"
                        \"\${CMAKE_INSTALL_PREFIX}/${args_DESTINATION}/$<TARGET_FILE_NAME:${target}>\"
                        --no-compiler-runtime
                        $<$<CONFIG:Debug>:--debug>
                        $<$<CONFIG:Release>:--release>)
                endif()
            ")
        else()
            add_dependencies(${target} qtpaths linuxdeployqt)
            _install(CODE "
                string(TOUPPER \"\${CMAKE_INSTALL_CONFIG_NAME}\" install_config)
                if (\"\${install_config}\" MATCHES \"${configurations}\")
                    file(RPATH_CHANGE
                        FILE \"\${CMAKE_INSTALL_PREFIX}/${args_DESTINATION}/$<TARGET_FILE_NAME:${target}>\"
                        OLD_RPATH \"$<TARGET_PROPERTY:${target},INSTALL_RPATH>\"
                        NEW_RPATH \"${QtWrapper_BINARY_DIR}/qtmodules/qtbase/lib:$<TARGET_PROPERTY:${target},INSTALL_RPATH>\")
                    execute_process(COMMAND \"$<TARGET_FILE:linuxdeployqt>\"
                        \"\${CMAKE_INSTALL_PREFIX}/${args_DESTINATION}/$<TARGET_FILE_NAME:${target}>\"
                        -unsupported-allow-new-glibc)
                endif()
            ")
        endif()
    endif()
endfunction()

function(qt_is_module target output)
    get_target_property(qt_version ${target} QT_TARGET_VERSION)
    if (qt_version)
        set(${output} on PARENT_SCOPE)
    else()
        set(${output} off PARENT_SCOPE)
    endif()
endfunction()
