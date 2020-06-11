#include <GUI/DOM/Helpers/ToggleButton.hpp> 

namespace ui::dom {

    ToggleButton::ToggleButton(bool value, const sf::Font& font, std::function<void(bool)> onChange, std::pair<String, String> labels)
        : CallbackButton(value ? labels.second : labels.first, font, [this](){
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

} // namespace ui::dom
