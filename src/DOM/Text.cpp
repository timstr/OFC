#include <OFC/DOM/Text.hpp> 

#include <cassert>

namespace ofc::ui::dom {

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
        const auto cs = static_cast<float>(m_text.getCharacterSize());
        const auto margin = std::round(cs * 0.25f);
        m_text.setPosition({margin, margin});
        setSize({bb.width + 2.0f * margin, cs + 2.0f * margin}, true);
        onChange();
    }

    const Font& Text::font() const {
        assert(m_text.getFont());
        return *m_text.getFont();
    }

    unsigned Text::characterSize() const {
        return m_text.getCharacterSize();
    }

    uint32_t Text::style() const {
        return m_text.getStyle();
    }

    Color Text::fillColor() const {
        return m_text.getFillColor();
    }

    Color Text::outlineColor() const {
        return m_text.getOutlineColor();
    }

    float Text::outlineThickness() const {
        return m_text.getOutlineThickness();
    }

    void Text::setFont(const Font& font){
        m_text.setFont(font);
    }

    void Text::setCharacterSize(unsigned s){
        m_text.setCharacterSize(s);
    }

    void Text::setStyle(uint32_t style){
        m_text.setStyle(style);
    }

    void Text::setFillColor(const Color& c){
        m_text.setFillColor(c);
    }

    void Text::setOutlineColor(const Color& c){
        m_text.setOutlineColor(c);
    }

    void Text::setOutlineThickness(float v){
        m_text.setOutlineThickness(v);
    }

    void Text::onChange(){

    }

    void Text::render(sf::RenderWindow& rw){
        rw.draw(m_text);
    }

    Text* Text::toText(){
        return this;
    }

} // namespace ofc::ui::dom
