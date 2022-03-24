add_library(imgui STATIC 
    imgui/imconfig.h
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    target_sources(imgui PRIVATE
        imgui/backends/imgui_impl_dx12.h
        imgui/backends/imgui_impl_dx12.cpp
        imgui/backends/imgui_impl_win32.h
        imgui/backends/imgui_impl_win32.cpp)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    target_sources(imgui PRIVATE
        imgui/backends/imgui_impl_metal.h
        imgui/backends/imgui_impl_metal.mm
        imgui/backends/imgui_impl_osx.h
        imgui/backends/imgui_impl_osx.mm)
else()
    target_sources(imgui PRIVATE
        imgui/backends/imgui_impl_vulkan.h
        imgui/backends/imgui_impl_vulkan.cpp
        imgui/backends/imgui_impl_sdl.h
        imgui/backends/imgui_impl_sdl.cpp)
endif()

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends)

set_target_properties(imgui PROPERTIES CXX_STANDARD 14)
if (APPLE)
    target_compile_options(imgui PRIVATE "-fobjc-arc")
endif()