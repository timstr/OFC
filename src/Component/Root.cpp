#include <OFC/Component/Root.hpp>

#include <OFC/DOM/Element.hpp>

namespace ofc::ui {

    std::unique_ptr<dom::Container> Root::mount(Window* window) {
        assert(window);
        assert(m_component);
        assert(!m_tempContainer);
        assert(!m_tempWindow);
        m_tempWindow = window;
        m_component->mount(this, nullptr);
        assert(m_tempContainer);
        assert(!m_tempWindow);
        return std::move(m_tempContainer);
    }

    void Root::unmount() {
        assert(m_component);
        m_component->unmount();
    }

    void Root::onInsertChildElement(std::unique_ptr<dom::Element> element, const Scope& /* scope */) {
        assert(!m_tempContainer);
        assert(m_tempWindow);
        auto c = element->toContainer();
        assert(c);
        c->m_parentWindow = std::exchange(m_tempWindow, nullptr);
        element.release();
        m_tempContainer = std::unique_ptr<dom::Container>(c);
    }

    void Root::onRemoveChildElement(dom::Element* /* whichElement */, const Component* /* whichDescendent */) {
        // Nothing to do
    }

    std::vector<const Component*> Root::getPossibleChildren() const noexcept {
        return { m_component.get() };
    }
    
} // namespace ofc::ui
