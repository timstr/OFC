#include <OFC/Component/PureComponent.hpp>

namespace ofc::ui {

    void PureComponent::onMount(const dom::Element* beforeElement) {
        m_component = render();
        m_component.tryMount(this, beforeElement);
    }

    void PureComponent::onUnmount() {
        m_component.tryUnmount();
    }

    std::vector<const Component*> PureComponent::getPossibleChildren() const noexcept {
        return { m_component.get() };
    }

} // namespace ofc::ui
