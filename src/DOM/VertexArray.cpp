#include <OFC/DOM/VertexArray.hpp>

namespace ofc::ui::dom {

    VertexArray::VertexArray()
        : m_primitiveType(sf::LineStrip) {

    }

    std::vector<sf::Vertex>& VertexArray::vertices() noexcept {
        return m_vertices;
    }

    const std::vector<sf::Vertex>& VertexArray::vertices() const noexcept {
        return m_vertices;
    }

    void VertexArray::setPrimitiveType(sf::PrimitiveType pt) noexcept {
        m_primitiveType = pt;
    }

    sf::PrimitiveType VertexArray::primitiveType() const noexcept {
        return m_primitiveType;
    }

    void VertexArray::render(sf::RenderWindow& rw) {
        rw.draw(m_vertices.data(), m_vertices.size(), m_primitiveType);
    }

} // namespace ofc::ui::dom
