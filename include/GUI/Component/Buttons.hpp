#pragma once

#include <GUI/Component/Component.hpp>
#include <GUI/Component/FontContext.hpp>
#include <GUI/Util/String.hpp>
#include <GUI/DOM/Helpers/CallbackButton.hpp>

namespace ui {

    class Button : public SimpleComponent<dom::CallbackButton>, public FontConsumer<Button> {
    public:
        Button(PropertyOrValue<String> s);

        Button& onClick(std::function<void()> f);

    private:
        Observer<String> m_caption;
        std::function<void()> m_onClick;

        std::unique_ptr<dom::CallbackButton> createElement() override final;

        void updateCaption(const String& s);

        void updateFont(const sf::Font*);
    };

} // namespace ui
