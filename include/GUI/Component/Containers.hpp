#pragma once

#include <GUI/Component/Component.hpp>

#include <GUI/DOM/FreeContainer.hpp>
#include <GUI/DOM/FlowContainer.hpp>

namespace ui {


    // NOTE: the primary template is deliberately undefined.
    // Full specializations should be created for each DOM container type
    template<typename ContainerType>
    class ContainerComponent;


    template<typename ContainerType>
    class ContainerComponentBase : public InternalComponent {
    public:
        ContainerComponentBase() : m_container(nullptr) {
            static_assert(std::is_base_of_v<dom::Container, ContainerType>, "ContainerType must derive from dom::Container");
        }

    protected:
        ContainerType* container() noexcept {
            return m_container;
        }
        const ContainerType* container() const noexcept {
            return m_container;
        }

        virtual std::unique_ptr<ContainerType> createContainer() = 0;

    private:
        ContainerType* m_container;

        virtual void onMountContainer(const dom::Element* beforeElement) = 0;
        virtual void onUnmountContainer() = 0;

        void onMount(const dom::Element* beforeElement) override final {
            assert(m_container == nullptr);
            auto cp = createContainer();
            assert(cp);
            m_container = cp.get();
            insertElement(std::move(cp), beforeElement);
            onMountContainer(beforeElement);
        }

        void onUnmount() override final {
            assert(m_container);
            onUnmountContainer();
            eraseElement(m_container);
            m_container = nullptr;
        }
    };

    template<>
    class ContainerComponent<dom::FreeContainer> : public ContainerComponentBase<dom::FreeContainer> {
    public:
        // TODO: allow any number of arguments
        ContainerComponent(AnyComponent c)
            : m_childComponent(std::move(c)) {

        }

    private:
        AnyComponent m_childComponent;

        std::unique_ptr<dom::FreeContainer> createContainer() override {
            return std::make_unique<dom::FreeContainer>();
        }

        void onMountContainer(const dom::Element* beforeElement) override final {
            m_childComponent.tryMount(this, beforeElement);
        }

        void onUnmountContainer() override final {
            m_childComponent.tryUnmount();
        }

        void onInsertChildElement(std::unique_ptr<dom::Element> element, const Component* /* whichDescendent */, const dom::Element* beforeElement) override final {
            auto c = container();
            assert(c);
            // TODO: insert element before `beforeElement`
            c->adopt(std::move(element));
        }

        void onRemoveChildElement(dom::Element* whichElement, const Component* /* whichDescendent */) override final {
            auto c = container();
            assert(c);
            c->release(whichElement);
        }

        std::vector<const Component*> getChildren() const noexcept override final {
            return {m_childComponent.get()};
        }
    };

    template<>
    class ContainerComponent<dom::FlowContainer> : public ContainerComponentBase<dom::FlowContainer> {
    public:
        // TODO: allow any number of arguments
        ContainerComponent(AnyComponent c)
            : m_childComponent(std::move(c)) {

        }

    private:
        AnyComponent m_childComponent;

        std::unique_ptr<dom::FlowContainer> createContainer() override {
            return std::make_unique<dom::FlowContainer>();
        }

        void onMountContainer(const dom::Element* beforeElement) override final {
            m_childComponent.tryMount(this, beforeElement);
        }

        void onUnmountContainer() override final {
            m_childComponent.tryUnmount();
        }

        // TODO: add some clean way to specify x/y position or alignment
        void onInsertChildElement(std::unique_ptr<dom::Element> element, const Component* /* whichDescendent */, const dom::Element* beforeElement) override final {
            auto c = container();
            assert(c);
            c->adopt(std::move(element), dom::LayoutStyle::Inline, beforeElement);
        }

        void onRemoveChildElement(dom::Element* whichElement, const Component* /* whichDescendent */) override final {
            auto c = container();
            assert(c);
            c->release(whichElement);
        }

        std::vector<const Component*> getChildren() const noexcept override final {
            return {m_childComponent.get()};
        }
    };

    // TODO: specialize other containers

} // namespace ui
