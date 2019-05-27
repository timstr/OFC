#include <GUI/Text.hpp> 

namespace ui {

    Text::Text(const String& str, const sf::Font& font, const Color& color, unsigned char_size, uint32_t style){
        m_text.setFont(font);
        m_text.setFillColor(color);
        m_text.setCharacterSize(char_size);
        m_text.setStyle(style);
        setText(str);
    }

    const String& Text::text() const {
        return m_text.getString();
    }

    void Text::setText(const String& str){
        m_text.setString(str);
        const auto bb = m_text.getLocalBounds();
        setSize({bb.width, bb.height});
    }

    void Text::render(sf::RenderWindow& rw){
        rw.draw(m_text);
    }

    Text* Text::toText(){
        return this;
    }

} // namespace ui
