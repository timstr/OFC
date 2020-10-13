#include <OFC/Component/Buttons.hpp>

namespace ofc::ui {

    Button::Button(Valuelike<String> s)
        : FontConsumer(&Button::updateFont)
        , m_caption(this, &Button::updateCaption, std::move(s)) {

    }

    Button& Button::onClick(std::function<void()> f) {
        m_onClick = std::move(f);
        return *this;
    }

    std::unique_ptr<dom::CallbackButton> Button::createElement() {
        return std::make_unique<dom::CallbackButton>(
            m_caption.getValuelike().getOnce(),
            *getFont().getValuelike().getOnce(),
            m_onClick
        );
    }

    void Button::updateCaption(const String& s) {
        element()->getCaption().setText(s);
    }

    void Button::updateFont(const sf::Font* f) {
        assert(f);
        element()->getCaption().setFont(*f);
    }
    
} // namespace ofc::ui
