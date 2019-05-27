#include <GUI/BoxElement.hpp> 

namespace ui {

    BoxElement::BoxElement(){
        m_rect.setSize(size());
    }
    Color BoxElement::borderColor() const {
        return m_rect.getOutlineColor();
    }
    Color BoxElement::backgroundColor() const {
        return m_rect.getFillColor();
    }
    void BoxElement::setBorderColor(const Color& color){
        m_rect.setOutlineColor(color);
    }
    void BoxElement::setBackgroundColor(const ui::Color& color){
        m_rect.setFillColor(color);
    }

    float BoxElement::borderRadius() const {
        return m_rect.getRadius();
    }

    float BoxElement::borderThickness() const {
        return m_rect.getOutlineThickness();
    }

    void BoxElement::setBorderRadius(float r){
        m_rect.setRadius(r);
    }

    void BoxElement::setBorderThickness(float t){
        m_rect.setOutlineThickness(t);
    }

    void BoxElement::render(sf::RenderWindow& rw){
        rw.draw(m_rect);
    }

    void BoxElement::onResize(){
        m_rect.setSize(size());
    }

} // namespace ui
