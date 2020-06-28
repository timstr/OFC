#pragma once

#include <GUI/Component/Property.hpp>

#include <GUI/DOM/Element.hpp>
#include <GUI/DOM/Container.hpp>
#include <GUI/DOM/FreeContainer.hpp>
#include <GUI/DOM/Text.hpp>

#include <memory>
#include <typeinfo>

namespace ui {

    // TODO: put template definitions into .tpp file

    class ComponentParent;

    class InternalComponent;

    class Component : public ObserverOwner {
    public:
        Component() noexcept;
        Component(Component&&) noexcept;
        virtual ~Component() noexcept;

        Component& operator=(Component&&) = delete;

        Component(const Component&) = delete;
        Component& operator=(const Component&) = delete;

        ComponentParent* parent() noexcept;
        const ComponentParent* parent() const noexcept;

        bool isMounted() const noexcept;

        void mount(ComponentParent* parent, const dom::Element* beforeSibling);
        void unmount();

        template<typename ContextProviderType>
        PropertyOrValue<typename ContextProviderType::ValueType>* findContext() noexcept {
            const auto p = parent();
            if (!p) {
                return nullptr;
            }
            if (auto pc = parent()->findContextProvider(typeid(ContextProviderType))) {
                return static_cast<PropertyOrValue<ContextProviderType::ValueType>*>(pc);
            }
            return nullptr;
        }

        dom::Element* getFirstElement() const noexcept;

        const Component* getNextComponent() const noexcept;

        virtual dom::Element* getElement() const noexcept;

    protected:
        void insertElement(std::unique_ptr<dom::Element> element, const dom::Element* beforeElement);
        void eraseElement(dom::Element* element);

        // Called when a component is first inserted. Should be overridden to insert whatever own elements
        // and/or to mount the desired child components
        virtual void onMount(const dom::Element* beforeSibling) = 0;

        // Called when a component is being removed. Should be overridden to clean up own elements and
        // unmount any active child components
        virtual void onUnmount() = 0;

    private:
        bool m_isMounted;
        ComponentParent* m_parent;

        ComponentParent* toComponentParent() noexcept;
        virtual const ComponentParent* toComponentParent() const noexcept;

        friend ObserverBase;

        friend ComponentParent;
    };

    // ComponentParent is the basic parent type to all components.
    // May be either an internal node or a root node
    class ComponentParent {
    public:
        ComponentParent() noexcept = default;
        ComponentParent(ComponentParent&&) noexcept = default;
        virtual ~ComponentParent() noexcept = default;

        ComponentParent& operator=(ComponentParent&&) = delete;

        ComponentParent(const ComponentParent&) = delete;
        ComponentParent& operator=(const ComponentParent&) = delete;

        /**
         * Inserts an element into the DOM on behalf of a child component. This method is only
         * intended to be called from a child component as in `parent()->insertChildElement(..., this, ...);`
         * - element            : the element to insert
         * - whichDescentdent    : The component which requested the insertion
         * - beforeElement        : (optional) a previously-inserted sibling element in the DOM before which the
         *                        new element will be inserted. Passing nullptr results in the new element simply
         *                         being appended to the rest of its siblings from the same component.
        */
        virtual void onInsertChildElement(std::unique_ptr<dom::Element> element, const Component* whichDescendent, const dom::Element* beforeElement) = 0;

        /**
         * 
         */
        virtual void onRemoveChildElement(dom::Element* whichElement, const Component* whichDescendent) = 0;

        virtual std::vector<const Component*> getChildren() const noexcept = 0;

    private:
        virtual void* findContextProvider(const std::type_info&) noexcept;

        friend Component;

        friend InternalComponent;

        template<typename T, typename D>
        friend class ContextProvider;
    };

    // Component producing any number of children and not directly
    // representing any DOM elements.
    class InternalComponent : public Component, public ComponentParent {
    public:

    private:
        void* findContextProvider(const std::type_info&) noexcept override;

        const ComponentParent* toComponentParent() const noexcept override final;
    };

    // Component representing a single dom::Element type that unconditionally
    // inserts that element when mounted, and contains no child components
    template<typename ElementType>
    class SimpleComponent : public Component {
    public:
        SimpleComponent() : m_element(nullptr) {
            static_assert(std::is_base_of_v<dom::Element, ElementType>, "ElementType must derive from dom::Element");
        }

    protected:
        ElementType* element() noexcept {
            return m_element;
        }
        const ElementType* element() const noexcept {
            return m_element;
        }

        virtual std::unique_ptr<ElementType> createElement() = 0;

    private:
        ElementType* m_element;

        void onMount(const dom::Element* beforeElement) override final {
            assert(m_element == nullptr);
            auto ep = createElement();
            assert(ep);
            m_element = ep.get();
            insertElement(std::move(ep), beforeElement);
        }

        void onUnmount() override final {
            assert(m_element);
            eraseElement(m_element);
            m_element = nullptr;
        }

        dom::Element* getElement() const noexcept override final {
            return m_element;
        }
    };

    // ForwardingComponent is an InternalComponent that always passes insert/remove
    // calls on to its parent
    class ForwardingComponent : public InternalComponent {
    public:


    private:
        void onInsertChildElement(std::unique_ptr<dom::Element> element, const Component* whichDescendent, const dom::Element* beforeElement) override final;

        void onRemoveChildElement(dom::Element* whichElement, const Component* whichDescendent) override final;
    };

    // To be used in public API methods where client can pass any component
    class AnyComponent final {
    public:
        AnyComponent() noexcept = default;
        AnyComponent(AnyComponent&&) noexcept;
        AnyComponent& operator=(AnyComponent&&) noexcept;
        ~AnyComponent() noexcept = default;

        AnyComponent(const AnyComponent&) = delete;
        AnyComponent& operator=(const AnyComponent&) = delete;

        // Boxes any component type
        template<
            typename ComponentType,
            typename std::enable_if_t<std::is_base_of_v<Component, std::decay_t<ComponentType>>>* = nullptr
        >
        AnyComponent(ComponentType&& c)
            : m_component(std::make_unique<std::decay_t<ComponentType>>(std::move(c))) {

        }

        // Creates a TextComponent
        AnyComponent(const Property<String>&);
        AnyComponent(String);
        AnyComponent(const char*); // TODO: support for other string literals

        // Returns whether the object is holding a component or not
        operator bool() const noexcept;

        Component* get() noexcept;
        const Component* get() const noexcept;

        Component* operator->() noexcept;
        const Component* operator->() const noexcept;

        void tryMount(InternalComponent* self, const dom::Element* beforeElement);

        void tryUnmount();

        bool isMounted() const noexcept;

        void reset();

    private:
        std::unique_ptr<Component> m_component;

        friend class InternalComponent;
    };

} // namespace ui
