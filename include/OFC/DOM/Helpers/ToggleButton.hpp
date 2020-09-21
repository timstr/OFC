#pragma once

#include <OFC/DOM/Helpers/CallbackButton.hpp>

namespace ofc::ui::dom {

    class ToggleButton : public CallbackButton {
    public:
        ToggleButton(bool value, const sf::Font& font, std::function<void(bool)> onChange, std::pair<String, String> labels = {"Off", "On"});

    private:

        bool m_value;
        std::function<void(bool)> m_onChange;
        std::pair<String, String> m_labels;
        //ui::Text& m_text;
    };

} // namespace ofc::ui::dom
