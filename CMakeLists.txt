cmake_minimum_required(VERSION 3.9)

project(tims-gui VERSION 0.1.0 DESCRIPTION "A simple and extensible GUI library using SFML")

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

set(tims-gui_headers
    include/GUI/GUI.hpp
    include/GUI/ProgramContext.hpp
    include/GUI/Window.hpp

    include/GUI/Component/All.hpp
    include/GUI/Component/Buttons.hpp
    include/GUI/Component/Component.hpp
    include/GUI/Component/Component.tpp
    include/GUI/Component/Containers.hpp
    include/GUI/Component/Context.hpp
    include/GUI/Component/FontContext.hpp
    include/GUI/Component/ForEach.hpp
    include/GUI/Component/If.hpp
    include/GUI/Component/List.hpp
    include/GUI/Component/MixedComponent.hpp
    include/GUI/Component/Property.hpp
    include/GUI/Component/Property.tpp
    include/GUI/Component/PureComponent.hpp
    include/GUI/Component/Root.hpp
    include/GUI/Component/StatefulComponent.hpp
    include/GUI/Component/Switch.hpp
    include/GUI/Component/Text.hpp
    include/GUI/Component/When.hpp

    include/GUI/Serialization/Serialization.hpp

    include/GUI/DOM/BoxElement.hpp
    include/GUI/DOM/Container.hpp
    include/GUI/DOM/Control.hpp
    include/GUI/DOM/Draggable.hpp
    include/GUI/DOM/Element.hpp
    include/GUI/DOM/FlowContainer.hpp
    include/GUI/DOM/FreeContainer.hpp
    include/GUI/DOM/GridContainer.hpp
    include/GUI/DOM/Image.hpp
    include/GUI/DOM/ListContainer.hpp
    include/GUI/DOM/Text.hpp
    include/GUI/DOM/TextEntry.hpp

    include/GUI/DOM/Helpers/CallbackButton.hpp
    include/GUI/DOM/Helpers/CallbackTextEntry.hpp
    include/GUI/DOM/Helpers/NumberTextEntry.hpp
    include/GUI/DOM/Helpers/PullDownMenu.hpp
    include/GUI/DOM/Helpers/ScrollPanel.hpp
    include/GUI/DOM/Helpers/Slider.hpp
    include/GUI/DOM/Helpers/ToggleButton.hpp

    include/GUI/Util/Color.hpp
    include/GUI/Util/Key.hpp
    include/GUI/Util/Pi.hpp
    include/GUI/Util/RoundedRectangle.hpp
    include/GUI/Util/String.hpp
    include/GUI/Util/TemplateMetaProgramming.hpp
    include/GUI/Util/UniqueAny.hpp
    include/GUI/Util/Vec2.hpp
)

set(tims-gui_srcs
    src/GUI.cpp
    src/ProgramContext.cpp
    src/Window.cpp

    src/Component/Buttons.cpp
    src/Component/Component.cpp
    src/Component/Containers.cpp
    src/Component/FontContext.cpp
    src/Component/If.cpp
    src/Component/List.cpp
    src/Component/MixedComponent.cpp
    src/Component/Property.cpp
    src/Component/PureComponent.cpp
    src/Component/Root.cpp
    src/Component/Text.cpp

    src/Serialization/Serialization.cpp

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

    src/DOM/Helpers/CallbackButton.cpp
    src/DOM/Helpers/CallbackTextEntry.cpp
    src/DOM/Helpers/NumberTextEntry.cpp
    src/DOM/Helpers/PullDownMenu.cpp
    src/DOM/Helpers/ScrollPanel.cpp
    src/DOM/Helpers/Slider.cpp
    src/DOM/Helpers/ToggleButton.cpp

    src/Util/Color.cpp
    src/Util/RoundedRectangle.cpp
    src/Util/UniqueAny.cpp
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
    target_compile_options(tims-gui PUBLIC
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
