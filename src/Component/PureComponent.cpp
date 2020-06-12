#include <GUI/Component/PureComponent.hpp>

namespace ui {

    void PureComponent::onMount(const dom::Element* beforeElement) {
        m_component = render();
        m_component.tryMount(this, beforeElement);
    }

    void PureComponent::onUnmount() {
        m_component.tryUnmount();
    }

    std::vector<const Component*> PureComponent::getChildren() const noexcept {
        return { m_component.get() };
    }

} // namespace ui
