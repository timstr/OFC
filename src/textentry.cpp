#include <GUI/TextEntry.hpp>

#include <GUI/Window.hpp>
#include <GUI/Context.hpp>
#include <GUI/Helpers/Pi.hpp>

#include <cassert>
#include <cctype>

namespace ui {

    // TODO: allow multiline? create a TextField class?

    TextEntry::TextEntry(const sf::Font& font, unsigned height)
        : Text("", font, {}, height, {})
        , m_cursorHead(0)
        , m_cursorTail(0)
        , m_overtype(false) {

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

    bool TextEntry::onLeftClick(int){
        const auto s = text();

        if (s.getSize() > 0){
            std::size_t min = static_cast<std::size_t>(-1);
            float minDist = 1e6f;
            const auto mp = localMousePos();
            for (std::size_t i = 0; i < s.getSize(); ++i){
                const auto charPos = m_text.findCharacterPos(i).x;
                const auto dist = std::abs(charPos - mp.x);
                if (dist < minDist){
                    minDist = dist;
                    min = i;
                }
            }

            assert(min != static_cast<std::size_t>(-1));

            m_cursorHead = min;
            if (!shift()){
                m_cursorTail = m_cursorHead;
            }
        }

        startTyping();
        return true;
    }

    void TextEntry::onType(){

    }

    void TextEntry::onReturn(){

    }

    bool TextEntry::validate() const {
        return true;
    }

    void TextEntry::handleBackspace(){
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
            if (ctrl()){
                skipLeft();
            } else {
                m_cursorHead -= 1;
            }
            const auto [i0, i1] = selection();
            const auto cut = s.substring(0, i0) + s.substring(i1);
            m_cursorHead = i0;
            m_cursorTail = m_cursorHead;
            setText(cut);
            handleChange();
        }
    }

    void TextEntry::handleDelete(){
        const auto s = text();

        // If there is a selection, just erase that
        if (m_cursorHead != m_cursorTail){
            const auto [i0, i1] = selection();
            const auto cut = s.substring(0, i0) + s.substring(i1);
            m_cursorHead = i0;
            m_cursorTail = m_cursorHead;
            setText(cut);
            handleChange();
            return;
        }

        if (m_cursorHead < s.getSize()){
            if (ctrl()){
                skipRight();
            } else {
                m_cursorHead += 1;
            }
            const auto cut = s.substring(0, m_cursorTail) + s.substring(m_cursorHead);
            m_cursorHead = m_cursorTail;
            setText(cut);
            handleChange();
        }
    }

    void TextEntry::handleLeft(){
        if (m_cursorHead > 0){
            if (ctrl()){
                skipLeft();
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
            if (ctrl()){
                skipRight();
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
        m_overtype = !m_overtype;
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
        const auto isSpace = [](sf::Uint32 ch){
            if (ch > std::numeric_limits<unsigned char>::max()){
                return false;
            }
            return std::isspace(static_cast<int>(ch)) != 0;
        };

        const auto s = text();
        auto pasted = sf::Clipboard::getString();
        auto last = std::remove_if(pasted.begin(), pasted.end(), isSpace);
        if (last != pasted.end()){
            pasted.erase(last - pasted.begin(), pasted.end() - last);
        }
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
            assert(m_cursorHead <= text().getSize());
            assert(m_cursorTail <= text().getSize());

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

            float width = 2.0f;
            if (m_overtype){
                if (m_cursorHead < text().getSize()){
                    width = m_text.findCharacterPos(m_cursorHead + 1).x - m_text.findCharacterPos(m_cursorHead).x;
                } else {
                    width = static_cast<float>(characterSize()) * 0.5f;
                }
            }

            rect.setSize({width, static_cast<float>(characterSize())});
            const auto t = Context::get().getProgramTime().asSeconds();
            const auto v = 0.5f + 0.5f * std::cos(t * pi<float> * 4.0f);
            rect.setFillColor(interpolate(0x0, 0xFF, v));

            rw.draw(rect);
        }
    }

    void TextEntry::onChange(){
        const auto l = text().getSize();
        m_cursorHead = std::clamp(m_cursorHead, 0ull, l);
        m_cursorTail = std::clamp(m_cursorTail, 0ull, l);
        onType();
    }

    TextEntry* TextEntry::toTextEntry(){
        return this;
    }

    void TextEntry::type(uint32_t unicode){
        // TODO: find out properly if character is printable
        if (unicode < 32 || unicode == 127){
            return;
        }

        auto str = text();

        if (m_cursorHead != m_cursorTail){
            const auto [i0, i1] = selection();

            const auto first = str.substring(0, i0);
            const auto mid = String{unicode};
            const auto rest = str.substring(i1);

            setText(first + mid + rest);
            m_cursorHead = i0 + 1;
            m_cursorTail = m_cursorHead;
        } else {
            if (m_overtype && m_cursorHead < str.getSize()){
                str[m_cursorHead] = unicode;
            } else {
                str.insert(m_cursorHead, {unicode});
            }
            m_cursorHead += 1;
            m_cursorTail = m_cursorHead;
            setText(str);
        }

        handleChange();
    }

    void TextEntry::handleChange(){
        // TODO: this ruins styling
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

    void TextEntry::skipLeft(){
        const auto s = text();
        if (s.getSize() == 0){
            return;
        }

        const auto doSkip = [&](bool whitespace){
            while (m_cursorHead > 0 && (whitespace != (s[m_cursorHead - 1] == ' '))){
                m_cursorHead -= 1;
            }
        };

        if (m_cursorHead == 0){
            return;
        }

        bool ws = s[m_cursorHead - 1] == ' ';
        if (!ws){
            doSkip(true);
        }
        doSkip(false);
    }

    void TextEntry::skipRight(){
        const auto s = text();
        if (s.getSize() == 0){
            return;
        }

        const auto doSkip = [&](bool whitespace){
            while (m_cursorHead < s.getSize() && (whitespace != (s[m_cursorHead] == ' '))){
                m_cursorHead += 1;
            }
        };

        if (m_cursorHead == s.getSize()){
            return;
        }

        bool ws = s[m_cursorHead] == ' ';
        if (!ws){
            doSkip(true);
        }
        doSkip(false);
    }

} // namespace ui
