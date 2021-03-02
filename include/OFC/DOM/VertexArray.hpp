#pragma once

#include <OFC/DOM/Element.hpp>

#include <SFML/Graphics.hpp>

namespace ofc::ui::dom {

    class VertexArray : public Element {
    public:
        VertexArray();

        std::vector<sf::Vertex>& vertices() noexcept;
        const std::vector<sf::Vertex>& vertices() const noexcept;

        void setPrimitiveType(sf::PrimitiveType) noexcept;
        sf::PrimitiveType primitiveType() const noexcept;

    private:
        void render(sf::RenderWindow& rw) override;

        std::vector<sf::Vertex> m_vertices;
        sf::PrimitiveType m_primitiveType;
    };

} // namespace ofc::ui::dom
