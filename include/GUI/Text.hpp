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

    class Text : virtual Element {
    public:
        Text(const std::wstring& str, sf::Font& font, unsigned char_size = 15, uint8_t style = TextStyle::Regular);
        ~Text();

        std::wstring text() const;
        void setText(const std::wstring&);

        const Font& font() const;
        unsigned characterSize() const;
        uint8_t style() const;
        const Color& fillColor() const;
        const Color& outlineColor() const;
        float outlineThickness() const;

        void setFont(const Font& font);
        void setCharacterSize(unsigned);
        void setStyle(uint8_t style);
        void setFillColor(const Color&);
        void setOutlineColor(const Color&);
        void setOutlineThickness(float);

    private:
        Text* toText() override;

    private:
        sf::Text m_text;
    };

} // namespace ui