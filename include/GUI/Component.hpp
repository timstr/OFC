#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <typeinfo>
#include <vector>

#include <GUI/Element.hpp>
#include <GUI/Container.hpp>
#include <GUI/FreeContainer.hpp>
#include <GUI/Text.hpp>
#include <GUI/Helpers/CallbackButton.hpp>

namespace ui {

    namespace detail {

        void unqueuePropertyUpdater(std::function<void()>);

        void updateAllProperties();

    } // namespace detail

    // TODO: put template definitions into .tpp file

    class Pinned {
    public:
        Pinned() noexcept = default;
        ~Pinned() noexcept = default;

        Pinned(const Pinned&) = delete;
        Pinned(Pinned&&) = delete;
        Pinned& operator=(Pinned&&) = delete;
        Pinned& operator=(const Pinned&) = delete;
    };

    template<typename T>
    class ListOfEdits {
    public:
        ListOfEdits(const std::vector<T>& vecOld, const std::vector<T>& vecNew) {
            // Compute longest common subsequence using dynamic-programming table
            auto oldBegin = std::size_t{0};
            auto newBegin = std::size_t{0};
            auto oldEnd = vecOld.size();
            auto newEnd = vecNew.size();
            while (oldBegin < oldEnd && newBegin < newEnd && vecOld[oldBegin] == vecNew[newBegin]) {
                ++oldBegin;
                ++newBegin;
            }
            while (oldEnd > oldBegin && newEnd > newBegin && vecOld[oldEnd - 1] == vecNew[newEnd - 1]) {
                --oldEnd;
                --newEnd;
            }
            assert(oldBegin <= oldEnd);
            assert(newBegin <= newEnd);
            const auto m = oldEnd - oldBegin;
            const auto n = newEnd - newBegin;
            auto table = std::vector<std::size_t>((n + 1) * (m + 1), 0);
            auto getTable = [&](std::size_t i, std::size_t j) -> decltype(auto) {
                assert(i <= m);
                assert(j <= n);
                return table[(j * m) + i];
            };
            {
                for (std::size_t i = 1; i <= m; ++i) {
                    const auto& vOld = vecOld[oldBegin + i - 1];
                    for (std::size_t j = 1; j <= n; ++j) {
                        const auto& vNew = vecNew[newBegin + j - 1];
                        getTable(i, j) = (vOld == vNew) ?
                            getTable(i - 1, j - 1) + 1:
                            std::max(getTable(i - 1, j), getTable(i, j - 1));
                    }
                }
            }
            // Traverse table in reverse to recover list of edits
            std::vector<Edit> edits;
            {
                for (std::size_t i = m, j = n;;){
                    if (i > 0 && j > 0 && (vecOld[oldBegin + i - 1] == vecNew[newBegin + j - 1])){
                        edits.push_back(Edit(EditType::Nothing));
                        --i;
                        --j;
                    } else if (j > 0 && (i == 0 || getTable(i, j - 1) >= getTable(i - 1, j))) {
                        edits.push_back(Edit(EditType::Insertion, vecNew[newBegin + j - 1]));
                        --j;
                    } else if (i > 0 && (j ==0 || getTable(i, j - 1) < getTable(i - 1, j))) {
                        edits.push_back(Edit(EditType::Deletion));
                        --i;
                    } else {
                        break;
                    }   
                }
            }
            for (std::size_t i = 0; i < newBegin; ++i) {
                m_edits.push_back(Edit(EditType::Nothing));
            }
            for (auto it = edits.rbegin(), itEnd = edits.rend(); it != itEnd; ++it) {
                m_edits.push_back(std::move(*it));
            }
            for (std::size_t i = newEnd; i < vecNew.size(); ++i) {
                m_edits.push_back(Edit(EditType::Nothing));
            }
        }

        class Edit;

        const std::vector<Edit>& getEdits() const noexcept {
            return m_edits;
        }

        enum EditType : std::uint8_t {
            Deletion,
            Insertion,
            Nothing
        };

        class Edit {
        public:
            const EditType type() const noexcept {
                return m_type;
            }

            bool deletion() const noexcept {
                return m_type == EditType::Deletion;
            }

            bool insertion() const noexcept {
                return m_type == EditType::Insertion;
            }

            bool nothing() const noexcept {
                return m_type == EditType::Nothing;
            }

            const T& value() const noexcept {
                assert(m_type == EditType::Insertion);
                assert(m_value.has_value());
                return *m_value;
            }

        private:
            Edit(EditType type, std::optional<T> value = std::nullopt)
                : m_type(type)
                , m_value(std::move(value)) {

            }

            const EditType m_type;
            const std::optional<T> m_value;

            friend ListOfEdits<T>;
        };

    private:
        std::vector<Edit> m_edits;
    };

    template<typename T>
    using CRefOrValue = std::conditional_t<std::is_scalar_v<T>, T, const T&>;

    template<typename T>
    struct Difference {
        // For scalar types, the difference argument type is simply that type with all top-level
        // CV-qualifications removed. This saves users from having to type the arguments of update
        // functions for simple pointers as `const SomeType* const&` when `const SomeType*` suffices
        // For other, more complicated types, the difference argument type is a reference to const T
        using ArgType = CRefOrValue<T>;

        static std::decay_t<T> compute(const T& /* vOld */, const T& vNew) noexcept {
            return vNew;
        }
    };

    template<typename T>
    struct Difference<std::vector<T>> {
        using ArgType = const ListOfEdits<T>&;

        static ListOfEdits<T> compute(const std::vector<T>& vOld, const std::vector<T>& vNew) noexcept {
            return ListOfEdits<T>{vOld, vNew};
        }
    };

    template<typename T>
    using DiffArgType = typename Difference<T>::ArgType;

    class Component;

    class ComponentParent;

    class ComponentRoot;

    class InternalComponent;

    class AnyComponent;

    template<typename T>
    class Property;

    template<typename T>
    class Observer;

    template<typename T>
    class Property : private Pinned {
    public:
        Property(const T& t) : m_value(t) {

        }
        Property(T&& t) : m_value(std::move(t)) {
        
        }

        ~Property() {
            for (auto& o : m_observers) {
                o->reset();
            }
        }

        const T& getOnce() const noexcept {
            return m_value;
        }

        T& getOnceMutable() noexcept {
            if (!m_previousValue.has_value()) {
                m_previousValue.emplace(static_cast<const T&>(m_value));
                registerForUpdate();
            }
            return m_value;
        }

        void set(const T& t) {
            if (!m_previousValue.has_value()) {
                m_previousValue.emplace(std::move(m_value));
                registerForUpdate();
            }
            m_value = t;
        }
        void set(T&& t) {
            if (!m_previousValue.has_value()) {
                m_previousValue.emplace(std::move(m_value));
                registerForUpdate();
            }
            m_value = std::move(t);
        }

        // TODO: operator= for const T& / T&&?

    private:
        T m_value;
        std::optional<T> m_previousValue;
        std::vector<Observer<T>*> m_observers;

        void purgeUpdates() {
            if (!m_previousValue.has_value()) {
                return;
            }
            if (m_previousValue == m_value) {
                m_previousValue.reset();
                return;
            }
            const auto diff = Difference<T>::compute(
                static_cast<const T&>(*m_previousValue),
                static_cast<const T&>(m_value)
            );
            m_previousValue.reset();
            for (auto& o : m_observers) {
                o->update(static_cast<const DiffArgType<T>&>(diff));
            }
        }

        void registerForUpdate() {
            detail::unqueuePropertyUpdater([this]() {
                purgeUpdates();
            });
        }

        friend Observer<T>;
    };

    template<typename T>
    class PropertyOrValue {
    public:
        // PropertyOrValue is implictly constructible from an l-value reference
        // to a matching property, in which case it will point to that property
        // and contain no fixed value.
        PropertyOrValue(Property<T>& target) noexcept
            : m_target(&target)
            , m_fixedValue(std::nullopt) {

        }

        // PropertyOrValue is implicitly constructible from anything that can be
        // used to construct a value of type T, in which case it will not point
        // to any property and will instead contain a fixed value.
        template<
            typename... Args,
            std::enable_if_t<
                !std::is_same_v<Property<T>&, Args...> &&
                std::is_constructible_v<T, Args...>
            >* = nullptr
        >
        PropertyOrValue(Args&&... args)
            : m_target(nullptr)
            , m_fixedValue(std::in_place, std::forward<Args>(args)...) {

        }

    private:
        Property<T>* m_target;
        std::optional<T> m_fixedValue;

        friend Observer<T>;
    };

    class ObserverBase {
    public:
        ObserverBase(Component* owner);
        ObserverBase(ObserverBase&&) noexcept;
        ObserverBase& operator=(ObserverBase&&);
        virtual ~ObserverBase();

        ObserverBase(const ObserverBase&) = delete;
        ObserverBase& operator=(const ObserverBase&&) = delete;

    protected:
        Component* owner() noexcept;
        const Component* owner() const noexcept;

    private:
        Component* m_owner;

        void addSelfTo(Component*);
        void removeSelfFrom(Component*);

        friend Component;
    };

    // TODO: allow passing a member function that takes its argument by value for
    // fundamental types and for pointers (e.g. to allow `const T*` instead of `const T* const&`
    // which is currently required).
    template<typename T>
    class Observer : public ObserverBase {
    public:
        template<typename ComponentType>
        Observer(ComponentType* self, void (ComponentType::* onUpdate)(DiffArgType<T>), Property<T>& target)
            : ObserverBase(self)
            , m_target(&target)
            , m_fixedValue(std::nullopt)
            , m_onUpdate(makeUpdateFunction(self, onUpdate)) {
            m_target->m_observers.push_back(this);
        }
        template<typename ComponentType>
        Observer(ComponentType* self, void (ComponentType::* onUpdate)(DiffArgType<T>), T fixedValue)
            : ObserverBase(self)
            , m_target(nullptr)
            , m_fixedValue(std::move(fixedValue))
            , m_onUpdate(makeUpdateFunction(self, onUpdate)) {

        }
        template<typename ComponentType>
        Observer(ComponentType* self, void (ComponentType::* onUpdate)(DiffArgType<T>), PropertyOrValue<T> pv)
            : ObserverBase(self)
            , m_target(nullptr)
            , m_fixedValue(std::nullopt)
            , m_onUpdate(makeUpdateFunction(self, onUpdate)) {
            assign(pv);
        }
        template<typename ComponentType>
        Observer(ComponentType* self, void (ComponentType::* onUpdate)(DiffArgType<T>))
            : ObserverBase(self)
            , m_target(nullptr)
            , m_fixedValue(std::nullopt)
            , m_onUpdate(makeUpdateFunction(self, onUpdate)) {

        }

        Observer(Observer&& o) noexcept 
            : ObserverBase(std::move(o))
            , m_target(std::exchange(o.m_target, nullptr))
            , m_fixedValue(std::exchange(o.m_fixedValue, std::nullopt))
            , m_onUpdate(std::exchange(o.m_onUpdate, nullptr)) {
            if (m_target) {
                auto& v = m_target->m_observers;
                assert(std::count(v.begin(), v.end(), &o) == 1);
                assert(std::count(v.begin(), v.end(), this) == 0);
                *std::find(v.begin(), v.end(), &o) = this;
            }
        }
        Observer& operator=(Observer&& o) {
            if (&o == this) {
                return *this;
            }
            reset();
            m_target = std::exchange(o.m_target, nullptr);
            m_fixedValue = std::exchange(o.m_fixedValue, std::nullopt);
            m_onUpdate = std::exchange(o.m_onUpdate, nullptr);
            if (m_target) {
                auto& v = m_target->m_observers;
                assert(std::count(v.begin(), v.end(), &o) == 1);
                assert(std::count(v.begin(), v.end(), this) == 0);
                *std::find(v.begin(), v.end(), &o) = this;
            }
            return *this;
        }
        ~Observer() {
            reset();
        }

        Observer(const Observer&) = delete;
        Observer& operator=(const Observer&) = delete;

        void assign(Property<T>& target) {
            m_target = &target;
            m_fixedValue = std::nullopt;

            const auto& init = T{};
            // if target is dirty, bring this observer up to speed with its previous
            // value so that next update purge will be accurate.
            // Otherwise, if target is clean, bring this observer up to speed with
            // its current value.
            const auto diff = Difference<T>::compute(
                init,
                target.m_previousValue.has_value() ?
                    static_cast<const T&>(*target.m_previousValue) :
                    static_cast<const T&>(target.m_value)
            );
            update(diff);
            m_target->m_observers.push_back(this);
        }

        void assign(T fixedValue) {
            m_target = nullptr;
            const auto diff = Difference<T>::compute(
                static_cast<const T&>(*m_fixedValue),
                static_cast<const T&>(fixedValue)
            );
            m_fixedValue = std::move(fixedValue);
            update(diff);
        }

        void assign(const PropertyOrValue<T>& pv) {
            assert(pv.m_fixedValue.has_value() || pv.m_target);
            if (pv.m_target) {
                assign(*pv.m_target);
            } else {
                assign(static_cast<const T&>(*pv.m_fixedValue));
            }
        }

        bool hasTarget() const noexcept {
            return m_target;
        }

        bool hasFixedValue() const noexcept {
            return m_fixedValue.has_value();
        }

        bool hasValue() const noexcept {
            return hasTarget() || hasFixedValue();
        }

        const T& getValueOnce() const noexcept {
            assert(hasValue());
            if (m_target) {
                return m_target->getOnce();
            } else {
                return *m_fixedValue;
            }
        }

        Property<T>* target() noexcept {
            return m_target;
        }
        const Property<T>* target() const noexcept {
            return m_target;
        }

        void reset() {
            if (m_target) {
                auto& v = m_target->m_observers;
                assert(std::count(v.begin(), v.end(), this) == 1);
                auto it = std::find(v.begin(), v.end(), this);
                assert(it != v.end());
                v.erase(it);
            }
            m_target = nullptr;
            m_fixedValue.reset();
        }

        void update(DiffArgType<T> t) {
            assert(m_onUpdate);
            assert(owner());
            m_onUpdate(owner(), t);
        }

    private:
        Property<T>* m_target;
        std::optional<T> m_fixedValue;
        std::function<void(Component*, DiffArgType<T>)> m_onUpdate;

        template<typename ComponentType>
        static std::function<void(Component*, DiffArgType<T>)> makeUpdateFunction(ComponentType* self, void (ComponentType::* onUpdate)(DiffArgType<T>)) {
            static_assert(std::is_base_of_v<Component, ComponentType>, "ComponentType must derive from Component");
            // NOTE: self could be captured here, but is not, because it will become a
            // dangling pointer if the component is moved, which it will be anytime
            // it's passed to an AnyComponent
            return [onUpdate](Component* self, DiffArgType<T> t){
                assert(self);
                assert(dynamic_cast<ComponentType*>(self));
                auto sd = static_cast<ComponentType*>(self);
                if (sd->isMounted()) {
                    (sd->*onUpdate)(t);
                }
            };
        }

        friend Property<T>;
    };

    // Property that is a pure function of another property.
    // Useful for 
    // TODO: inheriting from Property<T> doesn't really make sense,
    // in terms of inheriting set() and m_value. If those could be moved
    // into a sibling class like PrimaryProperty (subject to some all-around
    // renaming), this could be neatly resolved.
    template<typename T, typename U>
    class DerivedProperty : public Property<T>, public Observer<U> {
    public:
        DerivedProperty(Property<U>&, std::function<T(const U&)>);
    };



    class Component {
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

        void mount(ComponentParent* parent, const Element* beforeSibling);
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

        Element* getFirstElement() const noexcept;

        const Component* getNextComponent() const noexcept;

        virtual Element* getElement() const noexcept;

    protected:
        void insertElement(std::unique_ptr<Element> element, const Element* beforeElement);
        void eraseElement(Element* element);

        // Called when a component is first inserted. Should be overridden to insert whatever own elements
        // and/or to mount the desired child components
        virtual void onMount(const Element* beforeSibling) = 0;

        // Called when a component is being removed. Should be overridden to clean up own elements and
        // unmount any active child components
        virtual void onUnmount() = 0;

    private:
        bool m_isMounted;
        ComponentParent* m_parent;
        std::vector<ObserverBase*> m_observers;

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
         * - element			: the element to insert
         * - whichDescentdent	: The component which requested the insertion
         * - beforeElement		: (optional) a previously-inserted sibling element in the DOM before which the
         *                        new element will be inserted. Passing nullptr results in the new element simply
         *   					  being appended to the rest of its siblings from the same component.
        */
        virtual void onInsertChildElement(std::unique_ptr<Element> element, const Component* whichDescendent, const Element* beforeElement) = 0;

        /**
         * 
         */
        virtual void onRemoveChildElement(Element* whichElement, const Component* whichDescendent) = 0;

        virtual std::vector<const Component*> getChildren() const noexcept = 0;

    private:
        virtual void* findContextProvider(const std::type_info&) noexcept;

        friend Component;

        friend InternalComponent;

        template<typename T, typename D>
        friend class ContextProvider;
    };

    // Component producing any number of children?
    // Possibly representing a dom::Container??????
    class InternalComponent : public Component, public ComponentParent {
    public:
        // TODO: ?

    private:
        void* findContextProvider(const std::type_info&) noexcept override;

        const ComponentParent* toComponentParent() const noexcept override final;
    };

    // Component representing a single Element type that unconditionally
    // inserts that element when mounted, and contains no child components
    template<typename ElementType>
    class SimpleComponent : public Component {
    public:
        SimpleComponent() : m_element(nullptr) {
            static_assert(std::is_base_of_v<Element, ElementType>, "ElementType must derive from Element");
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

        void onMount(const Element* beforeElement) override final {
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

        Element* getElement() const noexcept override final {
            return m_element;
        }
    };

    // ForwardingComponent is an InternalComponent that always passes insert/remove
    // calls on to its parent
    class ForwardingComponent : public InternalComponent {
    public:


    private:
        void onInsertChildElement(std::unique_ptr<Element> element, const Component* whichDescendent, const Element* beforeElement) override final;

        void onRemoveChildElement(Element* whichElement, const Component* whichDescendent) override final;
    };

    // TODO: add support for stateful components
    // i.e. components that have associated state which does not purely
    // depend on the component properties. Useful for things like:
    // - x/y position of draggable objects
    // - positions of scrollable contents
    // - oh, so many things, really
    // The ReactJS-style way of doing this would be to have something like
    // a StatefulComponent<StateType> base class, where StateType is an
    // aggregrate struct/class containing nothing but public Property<T> members.
    // Computing diffs over the entire state as ReactJS does is not necessary, since
    // re-rendering would already be tied to the individual properties within the state.
    // StatefulComponent<StateType> should probably rely upon a method like
    // virtual AnyComponent StatefulComponent<StateType>::render(const StateType state&) = 0;
    // whose return value is used in onMount, where that `state` parameter that is passed
    // is a reference to the component's own state member object.
    // Typically, such StatefulComponents will not accept any children, but may
    // choose to do so, for example, by accepting AnyComponent in their constructor
    // or named-parameter-idiom-style member functions.
    // Note that for this StatefulComponent idea to work properly with AnyComponent,
    // every such derived class must be able to be moved from. This probably means
    // storing the state inside a unique_ptr<StateType> and letting special member
    // function generation do the rest.


    template<typename T, typename DerivedContextProvider>
    class ContextProvider : public ForwardingComponent {
    public:
        ContextProvider(PropertyOrValue<T> pv)
            : m_propOrVal(std::move(pv)) {

        }

        DerivedContextProvider& with(AnyComponent c) {
            m_component = std::move(c);
            return *static_cast<DerivedContextProvider*>(this);
        }

        using ValueType = T;

    private:
        PropertyOrValue<T> m_propOrVal;
        AnyComponent m_component;

        void* findContextProvider(const std::type_info& ti) noexcept override final {
            if (ti == typeid(DerivedContextProvider)) {
                return static_cast<void*>(&m_propOrVal);
            }
            if (auto p = parent()) {
                return p->findContextProvider(ti);
            }
            return nullptr;
        }

        void onMount(const Element* beforeElement) override final {
            m_component.tryMount(this, beforeElement);
        }

        void onUnmount() override final {
            m_component.tryUnmount();
        }

        std::vector<const Component*> getChildren() const noexcept override final {
            return {m_component.get()};
        }
    };

    template<typename Derived, typename ContextProviderType>
    class ContextConsumer {
    public:
        using ValueType = typename ContextProviderType::ValueType;

        ContextConsumer(void (Derived::* onUpdate)(DiffArgType<ValueType>))
            : m_observer(static_cast<Derived*>(this), onUpdate)
            , m_init(false) {

        }

    protected:
        Observer<ValueType>& getObserver() {
            if (!m_init) {
                if (m_observer.hasValue()) {
                    m_init = true;
                    return m_observer;
                } else if (auto pv = static_cast<Derived*>(this)->findContext<ContextProviderType>()){
                    m_observer.assign(*pv);
                    m_init = true;
                    return m_observer;
                } else {
                    throw std::runtime_error("Attempted to use context which could not be found in component tree");
                }
            }
            return m_observer;
        }

    private:
        Observer<ValueType> m_observer;
        bool m_init;
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
        AnyComponent(Property<String>&);
        AnyComponent(String);
        AnyComponent(const char*); // TODO: support for other string literals

        // Returns whether the object is holding a component or not
        operator bool() const noexcept;

        Component* get() noexcept;
        const Component* get() const noexcept;

        Component* operator->() noexcept;
        const Component* operator->() const noexcept;

        void tryMount(InternalComponent* self, const Element* beforeElement);

        void tryUnmount();

        bool isMounted() const noexcept;

    private:
        std::unique_ptr<Component> m_component;

        friend class InternalComponent;
    };

    // NOTE: the primary template is deliberately undefined.
    // Full specializations should be created for each container type
    template<typename ContainerType>
    class ContainerComponent;

    class ComponentRoot : public ComponentParent {
    public:
        template<typename ContainerType, typename... ContainerComponentArgs>
        static ComponentRoot create(ContainerComponentArgs&&... args) {
            static_assert(
                std::is_base_of_v<Container, ContainerType>,
                "ContainerType must derive from Container"
            );
            static_assert(
                std::is_constructible_v<ContainerComponent<ContainerType>, ContainerComponentArgs...>,
                "ContainerComponent<ContainerType> must be constructible from the provided arguments"
            );
            // NOTE: tag type is used only to select correct container type,
            // since it's not possible to explicitly specify constructor
            // template arguments (ugh...)
            // See https://stackoverflow.com/questions/2786946/c-invoke-explicit-template-constructor
            using tag_t = ContainerType*;
            auto tag = tag_t{nullptr};
            return ComponentRoot(tag, std::forward<ContainerComponentArgs>(args)...);
        }

        /*
         * Creates a container element and populates it with all child components.
         * This container is intended to be placed at the root of the DOM tree.
         * It is important to call unmount() before this container is destroyed,
         * since various event listeners in various child components will assume
         * that their corresponding DOM elements remain alive for as long as they
         * are mounted.
         */
        std::unique_ptr<Container> mount();

        /**
         * Removes all child components and their elements from the mounted container
         */
        void unmount();

    private:
        std::unique_ptr<InternalComponent> m_component;
        std::unique_ptr<Container> m_tempContainer;

        template<typename ContainerType, typename... ContainerComponentArgs>
        ComponentRoot(ContainerType* /* dummy tag */, ContainerComponentArgs&&... args)
            : m_component(std::make_unique<ContainerComponent<ContainerType>>(
                std::forward<ContainerComponentArgs>(args)...
            )) {

        }

        void onInsertChildElement(std::unique_ptr<Element> element, const Component* whichDescendent, const Element* beforeElement) override final;

        void onRemoveChildElement(Element* whichElement, const Component* whichDescendent) override final;

        std::vector<const Component*> getChildren() const noexcept override final;
    };

    //////////////////////////////////////

    template<typename ContainerType>
    class ContainerComponentBase : public InternalComponent {
    public:
        ContainerComponentBase() : m_container(nullptr) {
            static_assert(std::is_base_of_v<Container, ContainerType>, "ContainerType must derive from Container");
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

        virtual void onMountContainer(const Element* beforeElement) = 0;
        virtual void onUnmountContainer() = 0;

        void onMount(const Element* beforeElement) override final {
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
    class ContainerComponent<FreeContainer> : public ContainerComponentBase<FreeContainer> {
    public:
        // TODO: allow any number of arguments
        ContainerComponent(AnyComponent c)
            : m_childComponent(std::move(c)) {

        }

    private:
        AnyComponent m_childComponent;

        std::unique_ptr<FreeContainer> createContainer() override {
            return std::make_unique<FreeContainer>();
        }

        void onMountContainer(const Element* beforeElement) override final {
            m_childComponent.tryMount(this, beforeElement);
        }

        void onUnmountContainer() override final {
            m_childComponent.tryUnmount();
        }

        void onInsertChildElement(std::unique_ptr<Element> element, const Component* /* whichDescendent */, const Element* beforeElement) override final {
            auto c = container();
            assert(c);
            // TODO: insert element before `beforeElement`
            c->adopt(std::move(element));
        }

        void onRemoveChildElement(Element* whichElement, const Component* /* whichDescendent */) override final {
            auto c = container();
            assert(c);
            c->release(whichElement);
        }

        std::vector<const Component*> getChildren() const noexcept override final {
            return {m_childComponent.get()};
        }
    };

    template<>
    class ContainerComponent<FlowContainer> : public ContainerComponentBase<FlowContainer> {
    public:
        // TODO: allow any number of arguments
        ContainerComponent(AnyComponent c)
            : m_childComponent(std::move(c)) {

        }

    private:
        AnyComponent m_childComponent;

        std::unique_ptr<FlowContainer> createContainer() override {
            return std::make_unique<FlowContainer>();
        }

        void onMountContainer(const Element* beforeElement) override final {
            m_childComponent.tryMount(this, beforeElement);
        }

        void onUnmountContainer() override final {
            m_childComponent.tryUnmount();
        }

        // TODO: add some clean way to specify x/y position or alignment
        void onInsertChildElement(std::unique_ptr<Element> element, const Component* /* whichDescendent */, const Element* beforeElement) override final {
            auto c = container();
            assert(c);
            c->adopt(std::move(element), LayoutStyle::Inline, beforeElement);
        }

        void onRemoveChildElement(Element* whichElement, const Component* /* whichDescendent */) override final {
            auto c = container();
            assert(c);
            c->release(whichElement);
        }

        std::vector<const Component*> getChildren() const noexcept override final {
            return {m_childComponent.get()};
        }
    };

    // TODO: specialize other containers

    //////////////////////////////////////

    class UseFont : public ContextProvider<const sf::Font*, UseFont> {
    public:
        UseFont(PropertyOrValue<const sf::Font*>);
    };

    template<typename Derived>
    class FontConsumer : ContextConsumer<Derived, UseFont> {
    public:
        FontConsumer(void (Derived::* onUpdate)(const sf::Font*))
            : ContextConsumer(onUpdate) {

        }

        Derived& font(PropertyOrValue<const sf::Font*> pv) {
            getObserver().assign(std::move(pv));
            return *this;
        }

    protected:
        Observer<const sf::Font*>& getFont() {
            return getObserver();
        }
    };

    class TextComponent : public SimpleComponent<Text>, public FontConsumer<TextComponent> {
    public:
        TextComponent(PropertyOrValue<String> s);

    private:
        Observer<String> m_stringObserver;

        std::unique_ptr<Text> createElement() override final;

        void updateString(const String& s);

        void updateFont(const sf::Font*);
    };

    class Button : public SimpleComponent<CallbackButton>, public FontConsumer<Button> {
    public:
        Button(PropertyOrValue<String> s);

        Button& onClick(std::function<void()> f);

    private:
        Observer<String> m_caption;
        std::function<void()> m_onClick;

        std::unique_ptr<CallbackButton> createElement() override final;

        void updateCaption(const String& s);

        void updateFont(const sf::Font*);
    };

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

        void onMount(const Element* beforeSibling) override final;

        void onUnmount() override final;

        std::vector<const Component*> getChildren() const noexcept override final;

        void updateCondition(bool b);
    };

    class List : public ForwardingComponent {
    public:
        template<typename... ComponentTypes>
        List(ComponentTypes&&... components) {
            static_assert((... && std::is_convertible_v<ComponentTypes, AnyComponent>), "All arguments to List must be convertible to AnyComponent");
            int dummy[sizeof...(ComponentTypes)] = {
                (m_components.emplace_back(std::forward<ComponentTypes>(components)), 0)...
            };
        }

    private:
        std::vector<AnyComponent> m_components;

        void onMount(const Element* beforeSibling) override final;

        void onUnmount() override final;

        std::vector<const Component*> getChildren() const noexcept override final;
    };

    template<typename T>
    class ForEach : public ForwardingComponent {
    public:
        ForEach(PropertyOrValue<std::vector<T>> pv)
            : m_observer(this, &ForEach::updateContents, pv) {

        }

        ForEach& Do(std::function<AnyComponent(CRefOrValue<T>, const std::size_t&)> f) {
            assert(f);
            m_fn = std::move(f);
            return *this;
        }

        ForEach& Do(std::function<AnyComponent(CRefOrValue<T>)> f) {
            assert(f);
            m_fn = [f = std::move(f)](CRefOrValue<T> v, const std::size_t& /* index */){
                return f(v);
            };
            return *this;
        }

    private:
        Observer<std::vector<T>> m_observer;
        std::function<AnyComponent(CRefOrValue<T>, const std::size_t&)> m_fn;
        std::vector<std::pair<AnyComponent, std::unique_ptr<std::size_t>>> m_components;

        void onMount(const Element* beforeSibling) override final {
            assert(m_fn);
            const auto& vals = m_observer.getValueOnce();
            m_components.reserve(vals.size());
            auto i = std::size_t{0};
            for (const auto& v : vals) {
                auto pi = std::make_unique<std::size_t>(i);
                m_components.emplace_back(
                    m_fn(v, *pi),
                    std::move(pi)
                );
                ++i;
            }
            for (auto& c : m_components) {
                c.first.tryMount(this, beforeSibling);
            }
        }

        void onUnmount() override final {
            for (auto it = m_components.rbegin(), itEnd = m_components.rend(); it != itEnd; ++it) {
                it->first.tryUnmount();
            }
        }

        std::vector<const Component*> getChildren() const noexcept override final {
            auto ret = std::vector<const Component*>();
            ret.reserve(m_components.size());
            for (const auto& c : m_components) {
                ret.push_back(c.first.get());
            }
            return ret;
        }

        void updateContents(const ListOfEdits<T>& edits) {
            assert(m_fn);
            auto it = m_components.begin();
            auto i = std::size_t{0};
            for (const auto& e : edits.getEdits()) {
                if (e.insertion()) {
                    auto pi = std::make_unique<std::size_t>(i);
                    auto nextComp = (it == m_components.end()) ? getNextComponent() : it->first.get();
                    auto nextElement = nextComp ? nextComp->getFirstElement() : nullptr;
                    it = m_components.emplace(
                        it,
                        m_fn(e.value(), *pi),
                        std::move(pi)
                    );
                    it->first.tryMount(this, nextElement);
                    ++it;
                    ++i;
                } else if (e.deletion()) {
                    assert(it != m_components.end());
                    it->first.tryUnmount();
                    assert(it != m_components.end());
                    it = m_components.erase(it);
                } else {
                    assert(it != m_components.end());
                    assert(e.nothing());
                    assert(it != m_components.end());
                    *it->second = i;
                    ++it;
                    ++i;
                }
            }
        }
    };

    template<typename T>
    ForEach(Property<std::vector<T>>&) -> ForEach<T>;

    template<typename T>
    ForEach(std::vector<T>&&) -> ForEach<T>;

    template<typename T>
    ForEach(const std::vector<T>&) -> ForEach<T>;

    // TODO: Switch(x).Case(val1, anycomp).Case(val2, anycomp).Default(anycomp)
    // (will require x's value type to support == and move-construction

    // TODO: safe dereferencing of pointers/optionals
    // This will be very similar to ForEach, but just needs an appropriate name
    // e.g. WhenHasValue(Property<const T*>).then([](const T& t){ return SomeComponent(t); })
    // or WhenHasValue(Property<std::optional<T>>).then([](const T& t){ return SomeComponent(t); })
    // Naming ideas:
    // - When, Whenever, Maybe, Possibly, Perhaps, IfDeferred

} // namespace ui
