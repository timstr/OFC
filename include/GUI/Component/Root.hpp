#pragma once

#include <GUI/Component/Containers.hpp>

namespace ui {

    class Root : public ComponentParent {
    public:
        template<typename ContainerComponentType, typename... ContainerComponentArgs>
        static Root create(ContainerComponentArgs&&... args) {
            static_assert(
                std::is_base_of_v<ContainerComponent, ContainerComponentType>,
                "ContainerContainerType must derive from ContainerComponent"
            );
            static_assert(
                std::is_constructible_v<ContainerComponentType, ContainerComponentArgs...>,
                "ContainerComponentType must be constructible from the provided arguments"
            );
            // NOTE: tag type is used only to select correct container type,
            // since it's not possible to explicitly specify constructor
            // template arguments (ugh...)
            // See https://stackoverflow.com/questions/2786946/c-invoke-explicit-template-constructor
            using tag_t = ContainerComponentType*;
            auto tag = tag_t{nullptr};
            return Root(tag, std::forward<ContainerComponentArgs>(args)...);
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
        std::unique_ptr<InternalComponent> m_component;
        std::unique_ptr<dom::Container> m_tempContainer;

        template<typename ContainerComponentType, typename... ContainerComponentArgs>
        Root(ContainerComponentType* /* dummy tag */, ContainerComponentArgs&&... args)
            : m_component(std::make_unique<ContainerComponentType>(
                std::forward<ContainerComponentArgs>(args)...
            )) {

        }

        void onInsertChildElement(std::unique_ptr<dom::Element> element, const Scope& scope) override final;

        void onRemoveChildElement(dom::Element* whichElement, const Component* whichDescendent) override final;

        std::vector<const Component*> getChildren() const noexcept override final;
    };

} // namespace ui
