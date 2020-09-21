#include <OFC/Component/Text.hpp>

namespace ofc::ui {

    TextComponent::TextComponent(Valuelike<String> s)
        : FontConsumer(&TextComponent::updateFont)
        , m_stringObserver(this, &TextComponent::updateString, std::move(s)) {

    }

    std::unique_ptr<dom::Text> TextComponent::createElement() {
        return std::make_unique<dom::Text>(
            m_stringObserver.getValueOnce(),
            *getFont().getValueOnce(),
            sf::Color::Black
        );
    }

    void TextComponent::updateString(const String& s) {
        element()->setText(s);
    }

    void TextComponent::updateFont(const sf::Font* f) {
        assert(f);
        element()->setFont(*f);
    }
    
} // namespace ofc::ui
