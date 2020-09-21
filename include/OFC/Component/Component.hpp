#pragma once

#include <OFC/Observer.hpp>

#include <OFC/DOM/Element.hpp>
#include <OFC/DOM/Container.hpp>
#include <OFC/DOM/FreeContainer.hpp>
#include <OFC/DOM/Text.hpp>

#include <OFC/Serialization.hpp>

#include <memory>
#include <typeinfo>
#include <any>
#include <typeinfo>

namespace ofc::ui {

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
        Valuelike<typename ContextProviderType::ValueType>* findContext() noexcept {
            const auto p = parent();
            if (!p) {
                return nullptr;
            }
            if (auto pc = parent()->findContextProvider(typeid(ContextProviderType))) {
                return static_cast<Valuelike<ContextProviderType::ValueType>*>(pc);
            }
            return nullptr;
        }

        dom::Element* getFirstElement() const noexcept;

        const Component* getNextComponent() const noexcept;

        bool appearsBefore(const Component* other) const noexcept;

        virtual dom::Element* getElement() const noexcept;

        void serializeState(Serializer&) const;
        void deserializeState(Deserializer&);

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

        template<typename StateType, typename PersistenceType>
        friend class StatefulComponent;

        virtual void serializeStateImpl(Serializer&) const;
        virtual void deserializeStateImpl(Deserializer&) const;
    };

    class Scope {
    public:
        Scope(const Component* descendent, const dom::Element* beforeElement) noexcept;

        const Component* descendent() const noexcept;

        const dom::Element* beforeElement() const noexcept;

        /**
         * Adds some value or simply a tag to the scope.
         * If an item of the same type already exists,
         * nothing is done. This enables shadowing to
         * work intuitively, since innermost components
         * add items to the scope before outer components
         * see the same scope.
         * Examples:
         *  - the item could be a single empty trait class,
         *    like `class FlowBlock {}`, which is added per-component
         *    and for which the existence is checked in FlowContainerBase
         * - the item could be a simple struct like
         *   `class GridCellCoordinate { int x, int y };` which has a bit more
         *    information
         */

        template<typename T, typename... Args>
        void add(Args&&... args) {
            auto a = std::make_any<T>(std::forward<Args>(args)...);
            auto it = find_if(
                begin(m_items),
                end(m_items),
                [](const std::any& a) {
                    return a.type() == typeid(T);
                }
            );
            if (it != end(m_items)) {
                *it = std::move(a);
            }
            m_items.push_back(std::move(a));
        }

        template<typename T>
        bool has() const noexcept {
            auto it = find_if(
                begin(m_items),
                end(m_items),
                [](const std::any& a) {
                    return a.type() == typeid(T);
                }
            );
            return it != end(m_items);
        }

        template<typename T>
        const T* get() const noexcept {
            auto it = find_if(
                begin(m_items),
                end(m_items),
                [](const std::any& a) {
                    return a.type() == typeid(T);
                }
            );
            if (it != end(m_items)) {
                return std::any_cast<T>(&*it);
            }
            return nullptr;
        }

        std::size_t size() const noexcept;

    private:
        const Component* const m_descendent;
        const dom::Element* const m_beforeElement;
        std::vector<std::any> m_items;
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
        virtual void onInsertChildElement(std::unique_ptr<dom::Element> element, const Scope& scope) = 0;

        /**
         * 
         */
        virtual void onRemoveChildElement(dom::Element* whichElement, const Component* whichDescendent) = 0;

        virtual std::vector<const Component*> getChildren() const noexcept = 0;

    private:
        virtual void* findContextProvider(const std::type_info&) noexcept;

        Component* toComponent() noexcept;
        virtual const Component* toComponent() const noexcept;

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
        const Component* toComponent() const noexcept override final;
    };

    // Component representing a single dom::Element type that unconditionally
    // inserts that element when mounted, and contains no child components
    template<typename ElementType>
    class SimpleComponent : public Component {
    public:
        SimpleComponent() : m_element(nullptr) {
            static_assert(std::is_base_of_v<dom::Element, ElementType>, "ElementType must derive from dom::Element");
        }

        ElementType* element() const noexcept {
            return m_element;
        }

        dom::Element* getElement() const noexcept override final {
            return m_element;
        }

    protected:
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
    };

    // ForwardingComponent is an InternalComponent that always passes insert/remove
    // calls on to its parent
    class ForwardingComponent : public InternalComponent {
    public:
        void onInsertChildElement(std::unique_ptr<dom::Element> element, const Scope& scope) override;

        void onRemoveChildElement(dom::Element* whichElement, const Component* whichDescendent) override;
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

        // Creates a Text component
        AnyComponent(const Value<String>&);
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

    class SimpleForwardingComponent : public ForwardingComponent {
    public:
        SimpleForwardingComponent(AnyComponent);

    private:
        AnyComponent m_child;
        
        void onMount(const dom::Element*) override final;
        void onUnmount() override final;

        std::vector<const Component*> getChildren() const noexcept override final;
    };

    class Restorable : public InternalComponent {
    public:
        class Context {
        public:
            Context() noexcept;
            ~Context() noexcept;

            Context(Context&&) noexcept;
            Context& operator=(Context&&) noexcept;

            Context(const Context&) = delete;
            Context& operator=(const Context&) = delete;
            
            void save(Serializer&) const;

            void restore(Deserializer&);
        private:
            Restorable* m_restorable;

            friend Restorable;
        };

        
        Restorable(Context&);
        ~Restorable();

        Restorable(Restorable&&);
        
        Restorable() = delete;
        Restorable(const Restorable&) = delete;
        Restorable& operator=(Restorable&&) = delete;
        Restorable& operator=(const Restorable&&) = delete;

        Restorable& with(AnyComponent);

    private:
        AnyComponent m_component;
        Context* m_context;

        void onMount(const dom::Element* beforeSibling);

        void onUnmount();
    };

} // namespace ofc::ui
