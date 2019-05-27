#pragma once

#include <GUI/Element.hpp>
#include <GUI/RoundedRectangle.hpp>
#include <GUI/Color.hpp>

namespace ui {
    
    class BoxElement : virtual public Element {
    public:
        BoxElement();

        Color borderColor() const;
        Color backgroundColor() const;

        void setBorderColor(const Color&);
        void setBackgroundColor(const Color&);

        float borderRadius() const;
        float borderThickness() const;

        void setBorderRadius(float);
        void setBorderThickness(float);

        void render(sf::RenderWindow&) override;

        void onResize() override;

    private:
        RoundedRectangle m_rect;
    };

} // namespace ui