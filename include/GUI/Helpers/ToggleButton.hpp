#pragma once

#include <GUI/Control.hpp>
#include <GUI/Container.hpp>

namespace ui {

    class ToggleButton : public Control, public Container {
    public:
        ToggleButton(bool value, const sf::Font& font, std::function<void(bool)> on_change, std::pair<String, String> label = {"Off", "On"});
        // TODO
    };

} // namespace ui
