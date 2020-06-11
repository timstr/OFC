#pragma once

#include <GUI/Component/Component.hpp>

namespace ui {

    class If : public ForwardingComponent {
    public:
        If(Property<bool>& c);

        If& then(AnyComponent c);

        If& otherwise(AnyComponent c);

        // TODO: // add elseif(condition) method that adds another `If` component
        // as m_elseComponent and returns a reference to it

    private:
        Observer<bool> m_condition;
        AnyComponent m_thenComponent;
        AnyComponent m_elseComponent;

        void onMount(const dom::Element* beforeSibling) override final;

        void onUnmount() override final;

        std::vector<const Component*> getChildren() const noexcept override final;

        void updateCondition(bool b);
    };

} // namespace ui 
