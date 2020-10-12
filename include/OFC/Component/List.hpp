#pragma once

#include <OFC/Component/Component.hpp>

namespace ofc::ui {

    class List : public ForwardingComponent {
    public:
        template<typename... ComponentTypes>
        List(ComponentTypes&&... components) {
            static_assert((... && std::is_convertible_v<ComponentTypes, AnyComponent>), "All arguments to List must be convertible to AnyComponent");
            if constexpr (sizeof...(ComponentTypes) > 0){
                int dummy[] = {
                    (m_components.emplace_back(std::forward<ComponentTypes>(components)), 0)...
                };
                (void)dummy;
            }
        }

    private:
        std::vector<AnyComponent> m_components;

        void onMount(const dom::Element* beforeSibling) override final;

        void onUnmount() override final;

        std::vector<const Component*> getPossibleChildren() const noexcept override final;
    };

} // namespace ofc::ui 
