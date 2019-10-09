#include <GUI/Helpers/ToggleButton.hpp> 

namespace ui {

    ToggleButton::ToggleButton(bool value, const sf::Font& font, std::function<void(bool)> onChange, std::pair<String, String> labels)
        : CallbackButton(labels.first, font, [this](){
            m_value = !m_value;
            getCaption().setText(m_value ? m_labels.second : m_labels.first);
            if (m_onChange){
                m_onChange(m_value);
            }
        })
        , m_value(value)
        , m_onChange(std::move(onChange))
        , m_labels(std::move(labels))
        {
        
    }

    /*bool ToggleButton::onLeftClick(int clicks){
        m_value = !m_value;
        m_text.setText(m_value ? m_labels.second : m_labels.first);
        if (m_onChange){
            m_onChange(m_value);
        }
        return true;
    }

    ui::Text& ToggleButton::getText() noexcept {
        return m_text;
    }

    void ToggleButton::render(sf::RenderWindow& rw){
        BoxElement::render(rw);
        FlowContainer::render(rw);
    }*/

} // namespace ui
