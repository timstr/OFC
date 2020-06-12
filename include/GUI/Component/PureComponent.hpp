#pragma once

#include <GUI/Component/Component.hpp>

namespace ui {

    class PureComponent : public ForwardingComponent {
    protected:
        virtual AnyComponent render() const = 0;

    private:
        AnyComponent m_component;

        void onMount(const dom::Element* beforeElement) override final;

        void onUnmount() override final;

        std::vector<const Component*> getChildren() const noexcept override final;
    };

} // namespace ui
