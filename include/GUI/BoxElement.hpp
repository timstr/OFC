#pragma once

#include <GUI/Element.hpp>
#include <GUI/RoundedRectangle.hpp>

namespace ui {
    
    class BoxElement : virtual Element {
    public:

        sf::Color borderColor() const;
        sf::Color backgroundColor() const;

        void setBorderColor(sf::Color);
        void setBackgroundColor(sf::Color);

        float borderRadius() const;
        float borderThickness() const;

        void setBorderRadius(float);
        void setBorderThickness(float);

        void render(sf::RenderWindow&) override;

    private:
        RoundedRectangle m_rect;
    };

} // namespace ui