cmake_minimum_required(VERSION 3.9)

project(tims-gui VERSION 0.1.0 DESCRIPTION "A simple and extensible GUI library using SFML")

set(CMAKE_CXX_STANDARD 17)

if (UNIX)
    find_package(SFML 2.4 COMPONENTS system window graphics audio REQUIRED)
else()
    set(SFML_STATIC_LIBRARIES TRUE)
    find_package(SFML 2.4 COMPONENTS system window graphics audio main REQUIRED)
endif()


if (NOT SFML_FOUND)
	message(FATAL_ERROR "Could not find SFML. Please install SFML.")
endif()

set(tims-gui_headers
    include/GUI/BoxElement.hpp
    include/GUI/Color.hpp
	include/GUI/Container.hpp
	include/GUI/Context.hpp
	include/GUI/Control.hpp
    include/GUI/Draggable.hpp
	include/GUI/Element.hpp
	include/GUI/FlowContainer.hpp
	include/GUI/FreeContainer.hpp
	include/GUI/GridContainer.hpp
	include/GUI/GUI.hpp
	include/GUI/Helpers.hpp
    include/GUI/HorizontalList.hpp
	include/GUI/Image.hpp
	include/GUI/RoundedRectangle.hpp
	include/GUI/Text.hpp
	include/GUI/TextEntry.hpp
    include/GUI/VerticalList.hpp
	include/GUI/Window.hpp
	include/GUI/Helpers/CallbackButton.hpp
    include/GUI/Helpers/CallbackTextEntry.hpp
    include/GUI/Helpers/NumberTextEntry.hpp
    include/GUI/Helpers/Pi.hpp
    include/GUI/Helpers/PullDownMenu.hpp
    include/GUI/Helpers/ScrollPanel.hpp
    include/GUI/Helpers/Slider.hpp
    include/GUI/Helpers/ToggleButton.hpp
)

set(tims-gui_srcs
	src/BoxElement.cpp
    src/Color.cpp
	src/Container.cpp
	src/Context.cpp
	src/Control.cpp
    src/Draggable.cpp
	src/Element.cpp
	src/FlowContainer.cpp
	src/FreeContainer.cpp
	src/GridContainer.cpp
	src/GUI.cpp
    src/HorizontalList.cpp
	src/Image.cpp
	src/RoundedRectangle.cpp
	src/Text.cpp
	src/TextEntry.cpp
    src/VerticalList.cpp
	src/Window.cpp
	src/Helpers/CallbackButton.cpp
    src/Helpers/CallbackTextEntry.cpp
    src/Helpers/NumberTextEntry.cpp
    src/Helpers/PullDownMenu.cpp
    src/Helpers/ScrollPanel.cpp
    src/Helpers/Slider.cpp
    src/Helpers/ToggleButton.cpp
)
	
add_library(tims-gui STATIC ${tims-gui_headers} ${tims-gui_srcs})

if (UNIX)
    target_link_libraries(tims-gui
        PUBLIC sfml-system
        PUBLIC sfml-window
        PUBLIC sfml-graphics
        PUBLIC sfml-audio
    )
else()
    target_link_libraries(tims-gui
        PUBLIC sfml-system
        PUBLIC sfml-window
        PUBLIC sfml-graphics
        PUBLIC sfml-audio
        PUBLIC sfml-main
    )
endif()

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

if(MSVC)
	target_compile_options(tims-gui PUBLIC "/wd4250")
endif()
