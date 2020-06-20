#include <GUI/Component/If.hpp>

namespace ui {

    If::If(PropertyOrValue<bool> c)
        : m_condition(this, &If::updateCondition, std::move(c)) {

    }

    If& If::then(AnyComponent c) {
        m_thenComponent = std::move(c);
        return *this;
    }

    If& If::otherwise(AnyComponent c) {
        m_elseComponent = std::move(c);
        return *this;
    }

    void If::onMount(const dom::Element* beforeElement) {
        if (m_condition.getValueOnce()) {
            m_thenComponent.tryMount(this, beforeElement);
        } else {
            m_elseComponent.tryMount(this, beforeElement);
        }
    }

    void If::onUnmount() {
        m_thenComponent.tryUnmount();
        m_elseComponent.tryUnmount();
    }

    std::vector<const Component*> If::getChildren() const noexcept {
        return {m_condition.getValueOnce() ? m_thenComponent.get() : m_elseComponent.get()};
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
    
} // namespace ui
