#pragma once

#include <GUI/Component/Component.hpp>
#include <GUI/Component/FontContext.hpp>

#include <GUI/DOM/Text.hpp>

namespace ui {

    class TextComponent : public SimpleComponent<dom::Text>, public FontConsumer<TextComponent> {
    public:
        TextComponent(PropertyOrValue<String> s);

    private:
        Observer<String> m_stringObserver;

        std::unique_ptr<dom::Text> createElement() override final;

        void updateString(const String& s);

        void updateFont(const sf::Font*);
    };

} // namespace ui
