#pragma once

#include <OFC/Component/Component.hpp>
#include <OFC/Component/FontContext.hpp>

#include <OFC/DOM/Text.hpp>

namespace ofc::ui {

    class Text : public SimpleComponent<dom::Text>, public FontConsumer<Text> {
    public:
        Text(Valuelike<String> s);

    private:
        Observer<String> m_stringObserver;

        std::unique_ptr<dom::Text> createElement() override final;

        void updateString(const String& s);

        void updateFont(const sf::Font*);
    };


    class Span : public Component, public FontConsumer<Span> {
    public:
        Span(Valuelike<String> s);

    private:
        Observer<std::vector<String>> m_wordsObserver;
        std::vector<dom::Text*> m_words;

        void onMount(const dom::Element* beforeSibling) override;

        void onUnmount() override;

        void updateWords(const ListOfEdits<String>&);

        void updateFont(const sf::Font*);

        std::unique_ptr<dom::Text> makeWord(const String&);
    };

} // namespace ofc::ui
