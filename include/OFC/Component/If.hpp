#pragma once

#include <OFC/Component/Component.hpp>

namespace ofc::ui {

    class If : public ForwardingComponent {
    public:
        If(Value<bool> c);

        If& then(AnyComponent c);

        If& otherwise(AnyComponent c);

    private:
        Observer<bool> m_condition;
        AnyComponent m_thenComponent;
        AnyComponent m_elseComponent;

        void onMount(const dom::Element* beforeSibling) override final;

        void onUnmount() override final;

        std::vector<const Component*> getPossibleChildren() const noexcept override final;

        void updateCondition(bool b);
    };

} // namespace ofc::ui 
