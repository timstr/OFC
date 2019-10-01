#pragma once

#include <GUI/BoxElement.hpp>
#include <GUI/Control.hpp>
#include <GUI/FlowContainer.hpp>

namespace ui {

    class ToggleButton : public Control, public FlowContainer, public BoxElement {
    public:
        ToggleButton(bool value, const sf::Font& font, std::function<void(bool)> onChange, std::pair<String, String> labels = {"Off", "On"});

    private:
        bool onLeftClick(int clicks) override;

        void render(sf::RenderWindow&) override;

        ui::Text& getText() noexcept;

        bool m_value;
        std::function<void(bool)> m_onChange;
        std::pair<String, String> m_labels;
        ui::Text& m_text;
    };

} // namespace ui
