cmake_minimum_required(VERSION 3.9)

project(tims-gui VERSION 0.1.0 DESCRIPTION "A simple and extensible GUI library using SFML")

set(SFML_STATIC_LIBRARIES TRUE)
find_package(SFML 2.4 COMPONENTS system window graphics audio main REQUIRED)

if (NOT SFML_FOUND)
	message(FATAL_ERROR "Could not find SFML. Please install SFML.")
endif()

set(tims-gui_headers
	include/gui/context.h
	include/gui/doubleproperty.h
	include/gui/floatproperty.h
	include/gui/forms.h
	include/gui/formscontrol.h
	include/gui/formsmodel.h
	include/gui/gui.h
	include/gui/helpers.h
	include/gui/integerproperty.h
	include/gui/pulldownproperty.h
	include/gui/stringhelpers.h
	include/gui/stringproperty.h
	include/gui/text.h
	include/gui/textentry.h
	include/gui/transition.h
	include/gui/window.h
)

set(tims-gui_srcs
	src/context.cpp
	src/doubleproperty.cpp
	src/forms.cpp
	src/formscontrol.cpp
	src/formsmodel.cpp
	src/gui.cpp
	src/stringhelpers.cpp
	src/text.cpp
	src/textentry.cpp
	src/transition.cpp
	src/window.cpp
)
	
add_library(tims-gui STATIC ${tims-gui_headers} ${tims-gui_srcs})

target_link_libraries(tims-gui sfml-system sfml-window sfml-graphics sfml-audio sfml-main)

target_include_directories(tims-gui PUBLIC "include")
