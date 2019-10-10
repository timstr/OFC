#pragma once

#include <GUI/Element.hpp>
#include <GUI/Color.hpp>
#include <string>

namespace ui {
    
    enum TextStyle : uint8_t {
        Regular = sf::Text::Text::Style::Regular,
        Bold = sf::Text::Text::Style::Bold,
        Italic = sf::Text::Text::Style::Italic,
        Underlined = sf::Text::Text::Style::Underlined,
        StrikeThrough = sf::Text::Text::Style::StrikeThrough,
    };

    using Font = sf::Font;

    class Text : public virtual Element {
    public:
        Text(const String& str, const sf::Font& font, const Color& color = {}, unsigned char_size = 15, uint32_t style = TextStyle::Regular);

        const String& text() const;
        void setText(const String&);

        const Font& font() const;
        std::size_t characterSize() const;
        uint32_t style() const;
        Color fillColor() const;
        Color outlineColor() const;
        float outlineThickness() const;

        void setFont(const Font& font);
        void setCharacterSize(unsigned);
        void setStyle(uint32_t style);
        void setFillColor(const Color&);
        void setOutlineColor(const Color&);
        void setOutlineThickness(float);

        void render(sf::RenderWindow& rw) override;

    private:
        Text* toText() override;

        sf::Text m_text;

        friend class TextEntry;
    };

} // namespace ui