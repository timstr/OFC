cmake_minimum_required(VERSION 3.9)

project(ofc)

set(CMAKE_CXX_STANDARD 20)

if (UNIX)
    find_package(SFML 2.4 COMPONENTS system window graphics audio REQUIRED)
else()
    set(SFML_STATIC_LIBRARIES TRUE)
    find_package(SFML 2.4 COMPONENTS system window graphics audio main REQUIRED)
endif()

if (NOT SFML_FOUND)
    message(FATAL_ERROR "Could not find SFML. Please install SFML.")
endif()

set(ofc_headers
    include/OFC/UI.hpp
    include/OFC/ProgramContext.hpp
    include/OFC/Window.hpp
    include/OFC/Observer.hpp
    include/OFC/Serialization.hpp

    include/OFC/Component/All.hpp
    include/OFC/Component/Buttons.hpp
    include/OFC/Component/CheckBox.hpp
    include/OFC/Component/Commands.hpp
    include/OFC/Component/Component.hpp
    include/OFC/Component/Component.tpp
    include/OFC/Component/Containers.hpp
    include/OFC/Component/Context.hpp
    include/OFC/Component/FontContext.hpp
    include/OFC/Component/ForEach.hpp
    include/OFC/Component/If.hpp
    include/OFC/Component/List.hpp
    include/OFC/Component/MixedComponent.hpp
    include/OFC/Component/PullDown.hpp
    include/OFC/Component/PureComponent.hpp
    include/OFC/Component/Root.hpp
    include/OFC/Component/StatefulComponent.hpp
    include/OFC/Component/Slider.hpp
    include/OFC/Component/Switch.hpp
    include/OFC/Component/Text.hpp
    include/OFC/Component/TextField.hpp
    include/OFC/Component/VertexArray.hpp
    include/OFC/Component/When.hpp

    include/OFC/DOM/BoxElement.hpp
    include/OFC/DOM/Container.hpp
    include/OFC/DOM/Control.hpp
    include/OFC/DOM/Draggable.hpp
    include/OFC/DOM/Element.hpp
    include/OFC/DOM/FlowContainer.hpp
    include/OFC/DOM/FreeContainer.hpp
    include/OFC/DOM/GridContainer.hpp
    include/OFC/DOM/Image.hpp
    include/OFC/DOM/ListContainer.hpp
    include/OFC/DOM/Text.hpp
    include/OFC/DOM/TextEntry.hpp
    include/OFC/DOM/VertexArray.hpp

    include/OFC/Util/Color.hpp
    include/OFC/Util/Key.hpp
    include/OFC/Util/Pi.hpp
    include/OFC/Util/RoundedRectangle.hpp
    include/OFC/Util/String.hpp
    include/OFC/Util/TemplateMetaProgramming.hpp
    include/OFC/Util/UniqueAny.hpp
    include/OFC/Util/Vec2.hpp
)

set(ofc_srcs
    src/UI.cpp
    src/ProgramContext.cpp
    src/Observer.cpp
    src/Window.cpp
    src/Serialization.cpp

    src/Component/Buttons.cpp
    src/Component/CheckBox.cpp
    src/Component/Commands.cpp
    src/Component/Component.cpp
    src/Component/Containers.cpp
    src/Component/FontContext.cpp
    src/Component/If.cpp
    src/Component/List.cpp
    src/Component/MixedComponent.cpp
    src/Component/PureComponent.cpp
    src/Component/Root.cpp
    src/Component/Text.cpp
    src/Component/TextField.cpp
    src/Component/VertexArray.cpp

    src/DOM/BoxElement.cpp
    src/DOM/Container.cpp
    src/DOM/Control.cpp
    src/DOM/Draggable.cpp
    src/DOM/Element.cpp
    src/DOM/FlowContainer.cpp
    src/DOM/FreeContainer.cpp
    src/DOM/GridContainer.cpp
    src/DOM/Image.cpp
    src/DOM/ListContainer.cpp
    src/DOM/Text.cpp
    src/DOM/TextEntry.cpp
    src/DOM/VertexArray.cpp

    src/Util/Color.cpp
    src/Util/Key.cpp
    src/Util/RoundedRectangle.cpp
    src/Util/UniqueAny.cpp
)

add_library(ofc STATIC ${ofc_headers} ${ofc_srcs})

if (UNIX)
    target_link_libraries(ofc
        PUBLIC sfml-system
        PUBLIC sfml-window
        PUBLIC sfml-graphics
        PUBLIC sfml-audio
    )
else()
    target_link_libraries(ofc
        PUBLIC sfml-system
        PUBLIC sfml-window
        PUBLIC sfml-graphics
        PUBLIC sfml-audio
        PUBLIC sfml-main
    )
endif()

target_include_directories(ofc PUBLIC "include")

set(OFC_GENERATE_EXAMPLE OFF CACHE BOOL "When set to ON, the example target will be generated")

if(OFC_GENERATE_EXAMPLE)
    add_executable(ofc_example example/ofc_example.cpp)

    target_link_libraries(ofc_example
        PUBLIC ofc
    )

    set_property(TARGET ofc_example PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:ofc_example>")

    add_custom_command(TARGET ofc_example POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${CMAKE_CURRENT_SOURCE_DIR}/example/fonts/"
			"$<TARGET_FILE_DIR:ofc_example>/fonts"
	)
endif()

if(MSVC)
    target_compile_options(ofc PUBLIC
        # increase warning level
        "/W4"

        # Turn warning "not all control paths return a value" into an error
        "/we4715"

        # Disable warning "class X inherits Y::Z via dominance" because it is dumb and knows nothing of virtual inheritance
        "/wd4250"

        # use sane exception handling, rather than trying to catch segfaults and allowing resource leaks and UB.
        # Yes, MSVC is genuinely that stupid by default. See "Default exception handling behavior" at
        # https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model?view=vs-2019
        "/EHsc" 

        # turn warning C4239 (non-standard extension that allows temporaries to be bound to
        # non-const references, yay microsoft) into an error
        # This extension is without question the source of many bugs and disastrous surprises
        "/we4239"
    )
endif()
