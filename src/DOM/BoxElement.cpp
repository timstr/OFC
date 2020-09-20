#include <GUI/DOM/BoxElement.hpp> 

namespace ui::dom {

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
        m_rect.setRadius(std::max(0.0f, r));
    }

    void BoxElement::setBorderThickness(float t){
        t = std::max(0.0f, t);
        const auto s = size();
        m_rect.setSize({
            std::max(0.0f, s.x - t),
            std::max(0.0f, s.y - t)
        });
        m_rect.setOutlineThickness(t);
    }

    void BoxElement::render(sf::RenderWindow& rw){
        rw.draw(m_rect);
    }

    void BoxElement::onResize(){
        const auto s = size();
        const auto t = m_rect.getOutlineThickness();
        m_rect.setSize({
            std::max(0.0f, s.x - t),
            std::max(0.0f, s.y - t)
        });
    }

} // namespace ui::dom
