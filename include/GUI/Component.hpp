#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include <GUI/Element.hpp>
#include <GUI/Text.hpp>
#include <GUI/Helpers/CallbackButton.hpp>

namespace ui {

	// TODO: put template definitions into .tpp file

	template<typename T>
	class Property;

	template<typename T>
	class Observer;

	template<typename T>
	class Property {
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

		Property() = delete;
		Property(Property&&) = delete;
		Property(const Property&) = delete;
		Property& operator=(Property&&) = delete;
		Property& operator=(const Property&) = delete;

		const T& getOnce() const noexcept {
			return m_value;
		}

		void set(const T& t) {
			m_value = t;
			notifyObservers();
		}
		void set(T&& t) {
			m_value = std::move(t);
			notifyObservers();
		}

	private:
		T m_value;
		std::vector<Observer<T>*> m_observers;

		void notifyObservers() {
			for (auto& o : m_observers) {
				assert(o->m_onUpdate);
				o->m_onUpdate(static_cast<const T&>(m_value));
			}
		}
	};

	template<typename T>
	class PropertyOrValue {
	public:
		PropertyOrValue(Property<T>& target) noexcept
			: m_target(&target)
			, m_fixedValue(std::nullopt) {

		}

		PropertyOrValue(T fixedValue)
			: m_target(nullptr) {

		}

	private:
		Property<T>* m_target;
		std::optional<T> m_fixedValue;

		friend Observer<T>;
	};

	// TODO: allow observer to be given a single fixed value,
	// to avoid having to create additional dummy properties
	// for fixed inputs.
	// This should be done by adding a member std::optional<T>,
	// relevant constructor(s), relevent assign(...) method(s),
	// and adding a method like getOnce() that either gets the
	// stored value or refers to the targeted property
	template<typename T>
	class Observer {
	public:
		template<typename ComponentType>
		Observer(ComponentType* self, void (ComponentType::* onUpdate)(const T&), Property<T>& target)
			: m_target(&target)
			, m_fixedValue(std::nullopt)
			, m_onUpdate(makeUpdateFunction(self, onUpdate)) {

		}
		template<typename ComponentType>
		Observer(ComponentType* self, void (ComponentType::* onUpdate)(const T&), T fixedValue)
			: m_target(nullptr)
			, m_fixedValue(std::move(fixedValue))
			, m_onUpdate(makeUpdateFunction(self, onUpdate)) {

		}
		template<typename ComponentType>
		Observer(ComponentType* self, void (ComponentType::* onUpdate)(const T&))
			: m_target(nullptr)
			, m_fixedValue(std::nullopt)
			, m_onUpdate(makeUpdateFunction(self, onUpdate)) {

		}

		Observer(Observer&& o) noexcept 
			: m_target(std::exchange(o.m_target, nullptr))
			, m_fixedValue(std::exchange(o.m_fixedValue, std::nullopt))
			, m_onUpdate(std::exchange(o.m_onUpdate, nullptr)) {

		}
		Observer& operator=(Observer&& o) {
			reset();
			m_target = std::exchange(o.m_target, nullptr);
			m_fixedValue = std::exchange(o.m_fixedValue, std::nullopt);
			m_onUpdate = std::exchange(o.m_onUpdate, nullptr);
		}
		~Observer() {
			reset();
		}

		Observer(const Observer&) = delete;
		Observer& operator=(const Observer&) = delete;

		void assign(Property<T>& target) {
			m_target = &target;
			m_fixedValue = std::nullopt;
			assert(m_onUpdate);
			m_onUpdate(m_target->getOnce());
		}

		void assign(T fixedValue) {
			m_target = std::nullopt;
			m_fixedValue = std::move(fixedValue);
			assert(m_onUpdate);
			m_onUpdate(m_fixedValue);
		}

		void assign(PropertyOrValue<T> pv) {
			assert(pv.m_fixedValue.has_value() || pv.m_target);
			if (pv.m_target) {
				assign(pv.m_target);
			} else {
				assign(std::move(*pv.m_fixedValue));
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
			m_target = nullptr;
		}

	private:
		Property<T>* m_target;
		std::optional<T> m_fixedValue;
		std::function<void(const T&)> m_onUpdate;

		template<typename ComponentType>
		static std::function<void(const T&)> makeUpdateFunction(ComponentType* self, void (ComponentType::* onUpdate)(const T&)) {
			static_assert(std::is_base_of_v<Component, ComponentType>, "ComponentType must derive from Component");
			// TODO: batching updates (i.e. preventing multiple DOM edits before the same render call) could be done in here
			return [self, onUpdate](const T& t){
				if (self->isMounted()) {
					(self->*onUpdate)(t);
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

	// Actual important forward declarations
	class Component;
	class ComponentParent;
	class ComponentRoot;
	class InternalComponent;

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

		void mount(ComponentParent* parent);
		void unmount();

	protected:
		void insertElement(std::unique_ptr<Element> element, const Element* beforeElement = nullptr);
		void eraseElement(Element* element);

		// Called when a component is first inserted. Should be overridden to insert whatever own elements
		// and/or to mount the desired child components
		virtual void onMount() = 0;

		// Called when a component is being removed. Should be overridden to clean up own elements and
		// unmount any active child components
		virtual void onUnmount() = 0;

	private:
		bool m_isMounted;
		ComponentParent* m_parent;
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
	};

	class ComponentRoot : public ComponentParent {
	public:
		// TODO
		// AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

	private:
		void onInsertChildElement(std::unique_ptr<Element> element, const Component* whichDescendent, const Element* beforeElement) override final;

		void onRemoveChildElement(Element* whichElement, const Component* whichDescendent) override final;
	};

	// Component producing any number of children?
	// Possibly representing a dom::Container??????
	class InternalComponent : public Component, public ComponentParent {
	public:
		// TODO: ?

	};

	// Component representing a single Element type that unconditionally
	// inserts that element when mounted
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

		void onMount() override final {
			assert(m_element == nullptr);
			auto ep = createElement();
			assert(ep);
			m_element = ep.get();
			insertElement(std::move(ep));
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


	private:
		void onInsertChildElement(std::unique_ptr<Element> element, const Component* whichDescendent, const Element* beforeElement) override final;

		void onRemoveChildElement(Element* whichElement, const Component* whichDescendent) override final;
	};

	// To be used in public API methods where client can pass any component
	class AnyComponent final {
	public:
		AnyComponent() noexcept;

		// Boxes any component type
		template<
			typename ComponentType,
			typename std::enable_if_t<std::is_base_of_v<Component, std::decay_t<ComponentType>>>* = nullptr
		>
			AnyComponent(ComponentType&& c)
			: m_component(std::make_unique<std::decay_t<ComponentType>>(std::move(c))) {

		}

		// Returns whether the object is holding a component or not
		operator bool() const noexcept;

		Component* operator->() noexcept;
		const Component* operator->() const noexcept;

		void tryMount(InternalComponent* self);

		void tryUnmount();

	private:
		std::unique_ptr<Component> m_component;

		friend class InternalComponent;
	};

	template<typename ContainerType>
	class ContainerComponent : public InternalComponent {
	public:
		ContainerComponent() : m_container(nullptr) {
			static_assert(std::is_base_of_v<Container, ContainerType>, "ContainerType must derive from Container");
		}

	protected:
		ContainerType* element() noexcept {
			return m_container;
		}
		const ContainerType* element() const noexcept {
			return m_container;
		}

		virtual std::unique_ptr<ContainerType> createContainer() = 0;

	private:
		Container* m_container;

		void onMount() override final {
			assert(m_container == nullptr);
			auto cp = createContainer();
			assert(cp);
			m_container = cp.get();
			insertElement(std::move(cp));
		}

		void onUnmount() override final {
			assert(m_container);
			eraseElement(m_container);
			m_container = nullptr;
		}
	};

	//////////////////////////////////////


	class StaticText : public SimpleComponent<Text> {
	public:
		StaticText(const sf::Font& f, sf::String s = "");

		// TODO color property

	private:
		const sf::Font* const m_font;
		String m_string;

		std::unique_ptr<Text> createElement() override final;
	};

	class DynamicText : public SimpleComponent<Text> {
	public:
		DynamicText(const sf::Font& f, Property<String>& s);

	private:
		const sf::Font* const m_font;
		Observer<String> m_stringObserver;

		std::unique_ptr<Text> createElement() override final;

		void updateString(const String& s);
	};

	class Button : public SimpleComponent<CallbackButton> {
	public:
		Button(const ui::Font& f);

		Button& caption(Property<String>& s);

		Button& onClick(std::function<void()> f);

	private:
		sf::Font const* const m_font;
		Observer<String> m_caption;
		std::function<void()> m_onClick;

		std::unique_ptr<CallbackButton> createElement() override final;

		void updateCaption(const String& s);
	};

	class If : public ForwardingComponent {
	public:
		If(Property<bool>& c);

		If& then(AnyComponent c);

		If& otherwise(AnyComponent c);

	private:
		Observer<bool> m_condition;
		AnyComponent m_thenComponent;
		AnyComponent m_elseComponent;

		void onMount() override final;

		void onUnmount() override final;

		void updateCondition(const bool& b);
	};

} // namespace ui
