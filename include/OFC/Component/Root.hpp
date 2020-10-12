#pragma once

#include <OFC/Component/Containers.hpp>

namespace ofc::ui {

    class Root : public ComponentParent {
    public:
        template<typename ContainerComponentType>
        Root(ContainerComponentType& c)
            : m_component(std::make_unique<ContainerComponentType>(std::move(c))) {

            static_assert(
                std::is_base_of_v<ContainerComponent, ContainerComponentType>,
                "ContainerContainerType must derive from ContainerComponent"
            );
        }

        /*
        * Creates a container element and populates it with all child components.
        * This container is intended to be placed at the root of the DOM tree.
        * It is important to call unmount() before this container is destroyed,
        * since various event listeners in various child components will assume
        * that their corresponding DOM elements remain alive for as long as they
        * are mounted.
        */
        std::unique_ptr<dom::Container> mount();

        /**
        * Removes all child components and their elements from the mounted container
        */
        void unmount();

    private:
        std::unique_ptr<ContainerComponent> m_component;
        std::unique_ptr<dom::Container> m_tempContainer;

        void onInsertChildElement(std::unique_ptr<dom::Element> element, const Scope& scope) override final;

        void onRemoveChildElement(dom::Element* whichElement, const Component* whichDescendent) override final;

        std::vector<const Component*> getPossibleChildren() const noexcept override final;
    };

} // namespace ofc::ui
