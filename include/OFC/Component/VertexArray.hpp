#pragma once

#include <OFC/Component/Component.hpp>

#include <OFC/DOM/VertexArray.hpp>

namespace ofc {

    // HACK: see notes in VertexArray.cpp
    template<>
    struct Summary<sf::Vertex> {
        using Type = std::tuple<vec2, sf::Color, vec2>;

        static Type compute(const sf::Vertex& v) {
            return {v.position, v.color, v.texCoords};
        }
    };

} // namespace ofc

namespace ofc::ui {

    class VertexArray : public SimpleComponent<dom::VertexArray> {
    public:
        VertexArray();

        VertexArray&& vertices(Value<std::vector<sf::Vertex>>);

        VertexArray&& primitiveType(Value<sf::PrimitiveType>);

    private:
        Observer<std::vector<sf::Vertex>> m_verticesObserver;
        Observer<sf::PrimitiveType> m_primitiveTypeObserver;

        void onChangeVertices(const ListOfEdits<sf::Vertex>&);

        void onChangePrimitiveType(sf::PrimitiveType);

        std::unique_ptr<dom::VertexArray> createElement() override final;
    };

} // namespace ofc::ui
