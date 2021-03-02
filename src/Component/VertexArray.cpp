#include <OFC/Component/VertexArray.hpp>

namespace ofc::ui {

    VertexArray::VertexArray()
        : m_verticesObserver(this, &VertexArray::onChangeVertices)
        , m_primitiveTypeObserver(this, &VertexArray::onChangePrimitiveType) {

    }

    VertexArray&& VertexArray::vertices(Value<std::vector<sf::Vertex>> vertices) {
        m_verticesObserver.assign(std::move(vertices));
        return std::move(*this);
    }

    VertexArray&& VertexArray::primitiveType(Value<sf::PrimitiveType> pt) {
        m_primitiveTypeObserver.assign(std::move(pt));
        return std::move(*this);
    }

    void VertexArray::onChangeVertices(const ListOfEdits<sf::Vertex>& loe) {
        // HACK: lazily copying over all elements.
        // Suggested extensions to Value<T> to make this use case better:
        // - add an option for Value<T> that prevents diffing calculations.
        //   If such a Value<T> is ever modifyed or accessed as non-const,
        //   mark it simply as dirty but without storing its past state.
        element()->vertices() = loe.newValue();
    }

    void VertexArray::onChangePrimitiveType(sf::PrimitiveType pt) {
        element()->setPrimitiveType(pt);
    }

    std::unique_ptr<dom::VertexArray> VertexArray::createElement() {
        auto e = std::make_unique<dom::VertexArray>();
        e->vertices() = m_verticesObserver.getValue().getOnce();
        e->setPrimitiveType(m_primitiveTypeObserver.getValue().getOnce());
        return e;
    }

} // namespace ofc::ui
