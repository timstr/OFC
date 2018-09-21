cmake_minimum_required(VERSION 3.9)

project(tims-gui VERSION 0.1.0 DESCRIPTION "A simple and extensible GUI library using SFML")

set(SFML_STATIC_LIBRARIES TRUE)
find_package(SFML 2.4 COMPONENTS system window graphics audio main REQUIRED)

if (NOT SFML_FOUND)
	message(FATAL_ERROR "Could not find SFML. Please install SFML.")
endif()

set(tims-gui_headers
	include/GUI/Context.hpp
	include/GUI/Element.hpp
	include/GUI/GUI.hpp
	include/GUI/Helpers.hpp
	include/GUI/Image.hpp
	include/GUI/RoundedRectangle.hpp
	include/GUI/Text.hpp
	include/GUI/TextEntry.hpp
	include/GUI/Transition.hpp
	include/GUI/helpers/CallbackButton.hpp
	include/GUI/helpers/NumberTextEntry.hpp
	include/GUI/helpers/pi.hpp
	include/GUI/helpers/PullDownMenu.hpp
	include/GUI/helpers/ScrollPanel.hpp
	include/GUI/helpers/Slider.hpp
	include/GUI/helpers/StringHelpers.hpp
	include/GUI/helpers/TextEntryHelper.hpp
	include/GUI/helpers/ToggleButton.hpp
)

set(tims-gui_srcs
	src/Context.cpp
	src/GUI.cpp
	src/StringHelpers.cpp
	src/Text.cpp
	src/TextEntry.cpp
	src/Transition.cpp
	src/Element.cpp
	src/Image.cpp
	src/ToggleButton.cpp
	src/TextEntryHelper.cpp
	src/StringHelpers.cpp
	src/Slider.cpp
	src/ScrollPanel.cpp
	src/RoundedRectangle.cpp
	src/PullDownMenu.cpp
	src/NumberTextEntry.cpp
	src/CallbackButton.cpp
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

set(TIMS_GUI_GENERATE_EXAMPLE OFF CACHE BOOL "When set to ON, the example gui target will be generated")

if(TIMS_GUI_GENERATE_EXAMPLE)
	set(FONTS_PATH "${CMAKE_SOURCE_DIR}/example/fonts")
	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/example/fontspath.hpp.in"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/fontspath.hpp"
	)

	add_executable(tims-gui-example example/tims_gui_example.cpp example/fontspath.hpp)

	target_link_libraries(tims-gui-example
		PUBLIC tims-gui
	)
endif()