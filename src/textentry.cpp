#include <GUI/TextEntry.hpp>

#include <GUI/Window.hpp>
#include <GUI/Context.hpp>
#include <GUI/Helpers/Pi.hpp>

#include <cassert>

namespace ui {

    // TODO: allow multiline?

    TextEntry::TextEntry(const sf::Font& font, unsigned height)
        : Text("", font, {}, height, {})
        , m_cursorHead(0)
        , m_cursorTail(0) {

        setBackgroundColor(0xFFFFFFFF);
        //setBorderRadius(5.0f);
        setBorderColor(0x888888FF);
        setBorderThickness(2.0f);

        setMinSize({50.0f, static_cast<float>(height)});
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

    bool TextEntry::onLeftClick(int clicks){
        // TODO find cursor position
        startTyping();
        return true;
    }

    void TextEntry::onChange(){

    }

    void TextEntry::onReturn(){

    }

    bool TextEntry::validate() const {
        return true;
    }

    void TextEntry::handleBackspace(){
        // TODO: delete whole previous word if shift is down
        const auto s = text();


        // If there is a selection, just erase that
        if (m_cursorHead != m_cursorTail){
            const auto [i0, i1] = selection();
            const auto cut = s.substring(0, i0) + s.substring(i1);
            m_cursorTail = m_cursorHead;
            setText(cut);
            handleChange();
            return;
        }

        if (m_cursorHead > 0){
            m_cursorHead -= 1;
            m_cursorTail = m_cursorHead;
            const auto cut = s.substring(0, m_cursorHead) + s.substring(m_cursorHead + 1);
            setText(cut);
            handleChange();
        }
    }

    void TextEntry::handleDelete(){
        // TODO: delete whole following word if shift is down
        const auto s = text();

        // If there is a selection, just erase that
        if (m_cursorHead != m_cursorTail){
            const auto [i0, i1] = selection();
            const auto cut = s.substring(0, i0) + s.substring(i1);
            m_cursorTail = m_cursorHead;
            setText(cut);
            handleChange();
            return;
        }

        if (m_cursorHead < s.getSize()){
            const auto cut = s.substring(0, m_cursorHead) + s.substring(m_cursorHead + 1);
            setText(cut);
            handleChange();
        }
    }

    void TextEntry::handleLeft(){
        if (m_cursorHead > 0){
            const auto s = text();
            if (ctrl() && s.getSize() > 0 && m_cursorHead > 0){
                bool ws = s[m_cursorHead - 1] == ' ';
                if (!ws){
                    skipLeft(false);
                }
                skipLeft(true);
            } else {
                m_cursorHead -= 1;
            }
            if (!shift()){
                m_cursorTail = m_cursorHead;
            }
        }
    }

    void TextEntry::handleRight(){
        if (m_cursorHead < text().getSize()){
            const auto s = text();
            if (ctrl() && s.getSize() > 0 && m_cursorHead < s.getSize()){
                bool ws = s[m_cursorHead] == ' ';
                if (!ws){
                    skipRight(false);
                }
                skipRight(true);
            } else {
                m_cursorHead += 1;
            }
            if (!shift()){
                m_cursorTail = m_cursorHead;
            }
        }
    }

    void TextEntry::handleHome(){
        m_cursorHead = 0;
        if (!shift()){
            m_cursorTail = m_cursorHead;
        }
    }

    void TextEntry::handleEnd(){
        m_cursorHead = text().getSize();
        if (!shift()){
            m_cursorTail = m_cursorHead;
        }
    }

    void TextEntry::handleInsert(){
        // TODO: should insert/overtype mode be global or per-field?
    }

    void TextEntry::handleSelectAll(){
        m_cursorHead = text().getSize();
        m_cursorTail = 0;
    }

    void TextEntry::handleCopy(){
        if (m_cursorHead == m_cursorTail){
            return;
        }
        const auto s = text();
        const auto [i0, i1] = selection();
        const auto sub = s.substring(i0, i1);
        sf::Clipboard::setString(sub);
    }

    void TextEntry::handleCut(){
        if (m_cursorHead == m_cursorTail){
            return;
        }
        const auto s = text();
        const auto [i0, i1] = selection();
        const auto sub = s.substring(i0, i1);
        sf::Clipboard::setString(sub);
        setText(s.substring(0, i0) + s.substring(i1));
        m_cursorHead = i0;
        m_cursorTail = i0;
        handleChange();
    }

    void TextEntry::handlePaste(){
        // TODO: remove linebreaks and tabs

        const auto s = text();
        const auto pasted = sf::Clipboard::getString();
        const auto [i0, i1] = selection();
        const auto s1 = s.substring(0, m_cursorHead) + pasted + s.substring(m_cursorTail);
        m_cursorHead = i0 + pasted.getSize();
        m_cursorTail = m_cursorHead;
        setText(s1);
        handleChange();
    }

    void TextEntry::handleReturn(){
        if (validate()){
            onReturn();
        }
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

    void TextEntry::onLoseFocus(){
        if (isTyping()){
            stopTyping();
        }
    }

    void TextEntry::render(sf::RenderWindow& rw){
        BoxElement::render(rw);
        Text::render(rw);

        if (isTyping()){
            sf::RectangleShape rect;

            const auto posHead = m_text.findCharacterPos(m_cursorHead).x;
            const auto posTail = m_text.findCharacterPos(m_cursorTail).x;

            if (m_cursorHead != m_cursorTail){
                const auto pos0 = std::min(posHead, posTail);
                const auto pos1 = std::max(posHead, posTail);

                rect.setPosition({pos0, 0.0f});
                rect.setSize({pos1 - pos0, static_cast<float>(characterSize())});
                rect.setFillColor(sf::Color{0x0000FF40});
                rw.draw(rect);
            }

            rect.setPosition({posHead, 0.0f});
            rect.setSize({2.0, static_cast<float>(characterSize())});
            const auto t = Context::get().getProgramTime().asSeconds();
            const auto v = 0.5f + 0.5f * std::cos(t * pi<float> * 4.0f);
            rect.setFillColor(interpolate(0x0, 0xFF, v));

            rw.draw(rect);
        }

        // TODO: render selection

        // TODO: render box instead of bar if in overtype mode
    }

    TextEntry* TextEntry::toTextEntry(){
        return this;
    }

    void TextEntry::type(uint32_t unicode){
        // TODO: find out properly if character is printable
        if (unicode < 32 || unicode == 127){
            return;
        }

        const auto& str = text();

        const auto [i0, i1] = selection();

        const auto first = str.substring(0, i0);
        const auto mid = String{unicode};
        const auto rest = str.substring(i1);

        setText(first + mid + rest);
        m_cursorHead = i0 + 1;
        m_cursorTail = m_cursorHead;

        handleChange();
    }

    void TextEntry::handleChange(){
        setBackgroundColor(validate() ? 0xFFFFFFFF : 0xFF8888FF);
        onChange();
        setMinSize({50.0f, static_cast<float>(characterSize())});
    }

    std::pair<std::size_t, std::size_t> TextEntry::selection() const {
        return {
            std::min(m_cursorHead, m_cursorTail),
            std::max(m_cursorHead, m_cursorTail)
        };
    }

    bool TextEntry::shift() const {
        return keyDown(Key::LShift) || keyDown(Key::RShift);
    }

    bool TextEntry::ctrl() const {
        return keyDown(Key::LControl) || keyDown(Key::RControl);
    }

    void TextEntry::skipLeft(bool whitespace){
        const auto s = text();
        if (s.getSize() == 0){
            return;
        }
        while (m_cursorHead > 0 && (whitespace == (s[m_cursorHead - 1] == ' '))){
            m_cursorHead -= 1;
        }
    }

    void TextEntry::skipRight(bool whitespace){
        const auto s = text();
        if (s.getSize() == 0){
            return;
        }
        while (m_cursorHead < s.getSize() && (whitespace == (s[m_cursorHead] == ' '))){
            m_cursorHead += 1;
        }
    }

} // namespace ui
