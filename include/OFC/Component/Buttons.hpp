#pragma once

#include <OFC/Component/Component.hpp>
#include <OFC/Component/FontContext.hpp>
#include <OFC/Util/String.hpp>
#include <OFC/DOM/Helpers/CallbackButton.hpp>

namespace ofc::ui {

    class Button : public SimpleComponent<dom::CallbackButton>, public FontConsumer<Button> {
    public:
        Button(Valuelike<String> s);

        Button& onClick(std::function<void()> f);

    private:
        Observer<String> m_caption;
        std::function<void()> m_onClick;

        std::unique_ptr<dom::CallbackButton> createElement() override final;

        void updateCaption(const String& s);

        void updateFont(const sf::Font*);
    };

    // TODO: toggle button

} // namespace ofc::ui
