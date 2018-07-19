cmake_minimum_required(VERSION 3.9)

project(tims-gui VERSION 0.1.0 DESCRIPTION "A simple and extensible GUI library using SFML")

set(SFML_STATIC_LIBRARIES TRUE)
find_package(SFML 2.4 COMPONENTS system window graphics audio main REQUIRED)

if (NOT SFML_FOUND)
	message(FATAL_ERROR "Could not find SFML. Please install SFML.")
endif()

set(tims-gui_headers
	include/gui/context.h
	include/gui/gui.h
	include/gui/helpers.h
	include/gui/roundrectangle.h
	include/gui/stringhelpers.h
	include/gui/text.h
	include/gui/textentry.h
	include/gui/transition.h
	include/gui/element.h
	include/gui/image.h
	include/gui/helpers/CallbackButton.h
	include/gui/helpers/NumberTextEntry.h
	include/gui/helpers/PullDownMenu.h
	include/gui/helpers/Slider.h
	include/gui/helpers/TextEntryHelper.h
	include/gui/helpers/ToggleButton.h
	include/gui/helpers/pi.h
)

set(tims-gui_srcs
	src/context.cpp
	src/gui.cpp
	src/stringhelpers.cpp
	src/text.cpp
	src/textentry.cpp
	src/transition.cpp
	src/element.cpp
	src/image.cpp
)
	
add_library(tims-gui STATIC ${tims-gui_headers} ${tims-gui_srcs})

target_link_libraries(tims-gui
	PUBLIC sfml-system
	PUBLIC sfml-window
	PUBLIC sfml-graphics
	PUBLIC sfml-audio
	PUBLIC sfml-main
)

target_include_directories(tims-gui PUBLIC "include")

set(BUILD_EXAMPLE OFF CACHE BOOL "When set to ON, the example gui target will be built")

if(BUILD_EXAMPLE)
	set(FONTS_PATH "${CMAKE_SOURCE_DIR}/test/fonts")
	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/test/fontspath.h.in"
		"${CMAKE_CURRENT_SOURCE_DIR}/test/fontspath.h"
	)

	add_executable(tims-gui-example test/tims_gui_example.cpp test/fontspath.h)

	target_link_libraries(tims-gui-example
		PUBLIC tims-gui
	)
endif()