if (RUPKG_PLATFORM_AVR)
	return()
endif()

set(SwitchScriptClient_SOURCES
	src/Main.cpp
	src/MainWindow.ui src/MainWindow.cpp src/MainWindow.hpp
	src/CVSignalWidget.ui src/CVSignalWidget.cpp src/CVSignalWidget.hpp
	src/ByteCodeBuilder.cpp src/ByteCodeBuilder.hpp
	src/CompilerOld.cpp src/Compiler.hpp src/Compiler.cpp
	src/SSML.cpp src/SSML.hpp)
if (WIN32)
	add_executable(SwitchScriptClient WIN32 ${SwitchScriptClient_SOURCES})
elseif (APPLE)
	add_executable(SwitchScriptClient MACOSX_BUNDLE ${SwitchScriptClient_SOURCES})
else()
	add_executable(SwitchScriptClient ${SwitchScriptClient_SOURCES})
	rupkg_make_appimage(SwitchScriptClient)
endif()
target_link_libraries(SwitchScriptClient PRIVATE Qt6::Widgets Qt6::MultimediaWidgets SwitchScriptVM tinyxml2 tesseract opencv_core)

qt_deploy(SwitchScriptClient)

set_target_properties(SwitchScriptClient PROPERTIES
    AUTOUIC on
    AUTOUIC_EXECUTABLE $<TARGET_FILE:Qt6::uic>
    AUTOMOC on
    AUTOMOC_EXECUTABLE $<TARGET_FILE:Qt6::moc>
    AUTORCC on
    AUTORCC_EXECUTABLE $<TARGET_FILE:Qt6::rcc>
	CXX_STANDARD 17)

rupkg_mark_as_tool(SwitchScriptClient)
rupkg_install(PKG_TARGET SwitchScriptClient PROFILE "tool")
rupkg_strip(SwitchScriptClient)