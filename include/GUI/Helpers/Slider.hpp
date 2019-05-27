#pragma once

#include <GUI/Control.hpp>
#include <GUI/Container.hpp>

namespace ui {

    class Slider : public Control, public Container {
    public:
        Slider(float default_val, float min, float max, const sf::Font& font, std::function<void(float)> on_change);
        // TODO
    };

} // namespace ui
