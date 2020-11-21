#pragma once

#include <OFC/DOM/Element.hpp>
#include <OFC/Util/RoundedRectangle.hpp>
#include <OFC/Util/Color.hpp>

namespace ofc::ui::dom {
    
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

} // namespace ofc::ui::dom
