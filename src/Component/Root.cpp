#include <GUI/Component/Root.hpp>

#include <GUI/DOM/Element.hpp>

namespace ui {

    std::unique_ptr<dom::Container> Root::mount() {
        assert(m_component);
        assert(!m_tempContainer);
        m_component->mount(this, nullptr);
        assert(m_tempContainer);
        return std::move(m_tempContainer);
    }

    void Root::unmount() {
        assert(m_component);
        m_component->unmount();
    }

    void Root::onInsertChildElement(std::unique_ptr<dom::Element> element, const Scope& /* scope */) {
        assert(!m_tempContainer);
        auto c = element->toContainer();
        assert(c);
        element.release();
        m_tempContainer = std::unique_ptr<dom::Container>(c);
    }

    void Root::onRemoveChildElement(dom::Element* /* whichElement */, const Component* /* whichDescendent */) {
        // Nothing to do
    }

    std::vector<const Component*> Root::getChildren() const noexcept {
        return {m_component.get()};
    }
    
} // namespace ui
