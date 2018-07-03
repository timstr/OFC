cmake_minimum_required(VERSION 3.9)

project(tims_gui VERSION 0.1.0 DESCRIPTION "A simple and extensible GUI library using SFML")

set(SFML_STATIC_LIBRARIES TRUE)
find_package(SFML 2.4 COMPONENTS system window graphics audio main REQUIRED)

if (NOT SFML_FOUND)
	message(FATAL_ERROR "Could not find SFML. Please install SFML.")
endif()

set(headers
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
)

set(srcs
	src/context.cpp
	src/gui.cpp
	src/stringhelpers.cpp
	src/text.cpp
	src/textentry.cpp
	src/transition.cpp
	src/element.cpp
	src/image.cpp
)
	
add_library(tims_gui STATIC ${headers} ${srcs})

target_link_libraries(tims_gui sfml-system sfml-window sfml-graphics sfml-audio sfml-main)

target_include_directories(tims_gui PUBLIC "include")

# Testing below

set(FONTS_PATH "${CMAKE_SOURCE_DIR}/test/fonts")
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/test/fontspath.h.in"
	"${CMAKE_CURRENT_SOURCE_DIR}/test/fontspath.h"
)

add_executable(tims_gui_test test/test.cpp test/fontspath.h)

target_link_libraries(tims_gui_test
	PUBLIC tims_gui
)