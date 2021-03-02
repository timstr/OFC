#include <OFC/Component/If.hpp>

namespace ofc::ui {

    If::If(Value<bool> c)
        : m_condition(this, &If::updateCondition, std::move(c)) {

    }

    If&& If::then(AnyComponent c) {
        m_thenComponent = std::move(c);
        return std::move(*this);
    }

    If&& If::otherwise(AnyComponent c) {
        m_elseComponent = std::move(c);
        return std::move(*this);
    }

    void If::onMount(const dom::Element* beforeElement) {
        if (m_condition.getValue().getOnce()) {
            m_thenComponent.tryMount(this, beforeElement);
        } else {
            m_elseComponent.tryMount(this, beforeElement);
        }
    }

    void If::onUnmount() {
        m_thenComponent.tryUnmount();
        m_elseComponent.tryUnmount();
    }

    std::vector<const Component*> If::getPossibleChildren() const noexcept {
        return { m_thenComponent.get(), m_elseComponent.get() };
    }

    void If::updateCondition(bool b) {
        m_thenComponent.tryUnmount();
        m_elseComponent.tryUnmount();
        if (b) {
            m_thenComponent.tryMount(this, nullptr);
        } else {
            m_elseComponent.tryMount(this, nullptr);
        }
    }
    
} // namespace ofc::ui
