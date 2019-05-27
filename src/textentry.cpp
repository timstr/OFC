#include <GUI/TextEntry.hpp>

#include <GUI/Window.hpp>

#include <cassert>

namespace ui {

    TextEntry::TextEntry(sf::Font& font, unsigned height) :
        Text("", font, {}, height, {}){

    }

    void TextEntry::startTyping(){
        auto win = getParentWindow();
        assert(win);
        win->startTyping(this);
    }

    void TextEntry::stopTyping(){
        auto win = getParentWindow();
        assert(win);
        if (win->currentTextEntry() == this){
            win->stopTyping();
        }
    }

    bool TextEntry::isTyping() const {
        auto win = getParentWindow();
        assert(win);
        return win->currentTextEntry() == this;
    }

    void TextEntry::onType(uint32_t unicode){

    }

    void TextEntry::onReturn(){

    }

    bool TextEntry::validate() const {
        return true;
    }

    bool TextEntry::onKeyDown(Key key){
        if (key == Key::Enter){
            startTyping();
            return true;
        } else if (key == Key::Escape){
            stopTyping();
            return true;
        }
        return false;
    }

    TextEntry* TextEntry::toTextEntry(){
        return this;
    }

    void TextEntry::type(uint32_t unicode){
        const auto& str = text();

        const auto first = str.substring(0, m_cursor_begin);
        const auto mid = String(unicode);
        const auto rest = str.substring(m_cursor_end);

        setText(first + mid + rest);
        m_cursor_end = m_cursor_begin;
    }

} // namespace ui
