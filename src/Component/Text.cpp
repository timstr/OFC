#include <OFC/Component/Text.hpp>

namespace ofc::ui {

    Text::Text(Valuelike<String> s)
        : FontConsumer(&Text::updateFont)
        , m_stringObserver(this, &Text::updateString, std::move(s)) {

    }

    std::unique_ptr<dom::Text> Text::createElement() {
        return std::make_unique<dom::Text>(
            m_stringObserver.getValuelike().getOnce(),
            *getFont().getValuelike().getOnce(),
            sf::Color::Black
        );
    }

    void Text::updateString(const String& s) {
        element()->setText(s);
    }

    void Text::updateFont(const sf::Font* f) {
        assert(f);
        element()->setFont(*f);
    }
    
} // namespace ofc::ui
