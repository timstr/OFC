#include <GUI/Component.hpp>

namespace ui {


	namespace detail {

		std::vector<std::function<void()>>& propertyUpdateQueue() noexcept {
			static std::vector<std::function<void()>> theQueue;
			return theQueue;
		}

		void unqueuePropertyUpdater(std::function<void()> f) {
			auto& q = propertyUpdateQueue();
			q.push_back(std::move(f));
		}

		void updateAllProperties() {
			auto& q = propertyUpdateQueue();
			for (auto& f : q) {
				f();
			}
			q.clear();
		}
	}

    ObserverBase::ObserverBase(Component* owner)
        : m_owner(owner) {
        assert(m_owner);
        addSelfTo(m_owner);
    }

    ObserverBase::ObserverBase(ObserverBase&& o) noexcept
        : m_owner(std::exchange(o.m_owner, nullptr)) {
        
        if (m_owner){
            o.removeSelfFrom(m_owner);
            addSelfTo(m_owner);
        }
    }

    ObserverBase& ObserverBase::operator=(ObserverBase&& o) {
        if (&o == this) {
            return *this;
        }
        if (m_owner) {
            removeSelfFrom(m_owner);
        }
        if (o.m_owner) {
            o.removeSelfFrom(o.m_owner);
        }
        m_owner = std::exchange(o.m_owner, nullptr);
        if (m_owner) {
            addSelfTo(m_owner);
        }
        return *this;
    }

    Component* ObserverBase::owner() noexcept {
        return m_owner;
    }

    const Component* ObserverBase::owner() const noexcept {
        return m_owner;
    }

    void ObserverBase::addSelfTo(Component* c) {
        auto& v = c->m_observers;
        assert(std::count(v.begin(), v.end(), this) == 0);
        v.push_back(this);
    }

    void ObserverBase::removeSelfFrom(Component* c) {
        auto& v = c->m_observers;
        assert(std::count(v.begin(), v.end(), this) == 1);
        auto it = std::find(v.begin(), v.end(), this);
        assert(it != v.end());
        v.erase(it);
    }

    ObserverBase::~ObserverBase() {
        if (m_owner){
            removeSelfFrom(m_owner);
        }
    }



	Component::Component() noexcept
		: m_isMounted(false)
		, m_parent(nullptr) {

	}

	Component::Component(Component&& c) noexcept 
		: m_isMounted(false)
		, m_parent(nullptr)
        , m_observers(std::move(c.m_observers)) {
		assert(!c.m_isMounted);
		assert(!c.m_parent);
        for (auto o : m_observers) {
            assert(o->m_owner == &c);
            o->m_owner = this;
        }
	}

	Component::~Component() noexcept {
		// NOTE: unmounting here is not safe, since it would call a virtual function.
        // It may be assumed that all components will be unmounted before being destroyed.
        // Conversely, all components MUST unmount all their child components while
        // being unmounted.
		assert(!m_isMounted);

        // NOTE: it is expected that all own observers are somehow stored as members in
        // derived classes, and that they remove themselves from this vector when destroyed.
        assert(m_observers.empty());
	}

	ComponentParent* Component::parent() noexcept {
		return m_parent;
	}

	const ComponentParent* Component::parent() const noexcept {
		return m_parent;
	}

	bool Component::isMounted() const noexcept {
		return m_isMounted;
	}

	void Component::mount(ComponentParent* parent, const Element* beforeSibling) {
		assert(!m_isMounted);
		assert(!m_parent);
		m_parent = parent;
        onMount(beforeSibling);
		m_isMounted = true;
	}

	void Component::unmount() {
		assert(m_isMounted);
		assert(m_parent);
        m_isMounted = false;
		onUnmount();
		m_parent = nullptr;
	}

	Element* Component::getFirstElement() const noexcept {
		if (auto e = getElement()) {
			return e;
		}
		if (auto p = toComponentParent()){
			auto c = p->getChildren();
			for (const auto& cc : c) {
				if (auto e = cc->getFirstElement()) {
					return e;
				}
			}
		}
		return nullptr;
	}

	const Component* Component::getNextComponent() const noexcept {
		assert(m_parent);
		auto siblings = m_parent->getChildren();
		assert(std::count(siblings.begin(), siblings.end(), this) == 1);
		auto it = std::find(siblings.begin(), siblings.end(), this);
		assert(it != siblings.end());
		++it;
		return (it == siblings.end()) ? nullptr : *it;
	}

	Element* Component::getElement() const noexcept {
		return nullptr;
	}

	void Component::insertElement(std::unique_ptr<Element> element, const Element* beforeElement) {
		assert(m_parent);
		m_parent->onInsertChildElement(std::move(element), this, beforeElement);
	}

	void Component::eraseElement(Element* element) {
		assert(m_parent);
		m_parent->onRemoveChildElement(element, this);
	}

	ComponentParent* Component::toComponentParent() noexcept {
		return const_cast<ComponentParent*>(const_cast<const Component*>(this)->toComponentParent());
	}

	const ComponentParent* Component::toComponentParent() const noexcept {
		return nullptr;
	}



    std::unique_ptr<Container> ComponentRoot::mount() {
        assert(m_component);
        assert(!m_tempContainer);
        m_component->mount(this, nullptr);
        assert(m_tempContainer);
        return std::move(m_tempContainer);
    }

    void ComponentRoot::unmount() {
        assert(m_component);
        m_component->unmount();
    }

	void ComponentRoot::onInsertChildElement(std::unique_ptr<Element> element, const Component* whichDescendent, const Element* beforeElement) {
        assert(!m_tempContainer);
		auto c = element->toContainer();
		assert(c);
        element.release();
        m_tempContainer = std::unique_ptr<Container>(c);
	}

	void ComponentRoot::onRemoveChildElement(Element* whichElement, const Component* whichDescendent) {
		// Nothing to do
	}

	std::vector<const Component*> ComponentRoot::getChildren() const noexcept {
		return {m_component.get()};
	}

	void ForwardingComponent::onInsertChildElement(std::unique_ptr<Element> element, const Component* whichDescendent, const Element* beforeElement) {
		assert(parent());
		parent()->onInsertChildElement(std::move(element), whichDescendent, beforeElement);
	}

	void ForwardingComponent::onRemoveChildElement(Element* whichElement, const Component* whichDescendent) {
		assert(parent());
		parent()->onRemoveChildElement(whichElement, whichDescendent);
	}



	AnyComponent::AnyComponent(AnyComponent&& ac) noexcept
		: m_component(std::exchange(ac.m_component, nullptr)) {
        
	}

    AnyComponent& AnyComponent::operator=(AnyComponent&& ac) noexcept {
        if (&ac == this) {
            return *this;
        }
        assert(!m_component || !m_component->isMounted());
        m_component = std::exchange(ac.m_component, nullptr);
        return *this;
    }

    AnyComponent::AnyComponent(Property<String>& p)
        : m_component(std::make_unique<TextComponent>(p)) {
    }

    AnyComponent::AnyComponent(String s)
        : m_component(std::make_unique<TextComponent>(std::move(s))) {
    }

    AnyComponent::AnyComponent(const char* s)
        : AnyComponent(String(s)) {

    }

	Component* AnyComponent::get() noexcept {
		return m_component.get();
	}
	const Component* AnyComponent::get() const noexcept {
		return m_component.get();
	}

    AnyComponent::operator bool() const noexcept {
		return static_cast<bool>(m_component);
	}

	Component* AnyComponent::operator->() noexcept {
		assert(m_component);
		return m_component.get();
	}
	const Component* AnyComponent::operator->() const noexcept {
		assert(m_component);
		return m_component.get();
	}

	void AnyComponent::tryMount(InternalComponent* self, const Element* beforeElement) {
		if (m_component && !m_component->isMounted()){
			m_component->mount(self, beforeElement);
		}
	}

	void AnyComponent::tryUnmount() {
		if (m_component && m_component->isMounted()){
			m_component->unmount();
		}
	}

	bool AnyComponent::isMounted() const noexcept {
		return m_component ? m_component->isMounted() : false;
	}



    TextComponent::TextComponent(PropertyOrValue<String> s)
		: FontConsumer(&TextComponent::updateFont)
		, m_stringObserver(this, &TextComponent::updateString, std::move(s)) {

	}

	std::unique_ptr<Text> TextComponent::createElement() {
		return std::make_unique<Text>(
			m_stringObserver.getValueOnce(),
			*getFont().getValueOnce(),
			sf::Color::Black
		);
	}

	void TextComponent::updateString(const String& s) {
		element()->setText(s);
	}

    void TextComponent::updateFont(const sf::Font* f) {
        assert(f);
        element()->setFont(*f);
    }



	Button::Button()
		: FontConsumer(&Button::updateFont)
		, m_caption(this, &Button::updateCaption) {

	}

	Button& Button::caption(PropertyOrValue<String> s) {
		m_caption.assign(s);
		return *this;
	}

	Button& Button::onClick(std::function<void()> f) {
		m_onClick = std::move(f);
		return *this;
	}

	std::unique_ptr<CallbackButton> Button::createElement() {
		return std::make_unique<CallbackButton>(
			m_caption.getValueOnce(),
			*getFont().getValueOnce(),
			m_onClick
		);
	}

	void Button::updateCaption(const String& s) {
		element()->getCaption().setText(s);
	}

    void Button::updateFont(const sf::Font* f) {
        assert(f);
        element()->getCaption().setFont(*f);
    }



	If::If(Property<bool>& c)
		: m_condition(this, &If::updateCondition, c) {

	}

	If& If::then(AnyComponent c) {
		m_thenComponent = std::move(c);
		return *this;
	}

	If& If::otherwise(AnyComponent c) {
		m_elseComponent = std::move(c);
		return *this;
	}
	void If::onMount(const Element* beforeElement) {
		if (m_condition.getValueOnce()) {
			m_thenComponent.tryMount(this, beforeElement);
		} else {
			m_elseComponent.tryMount(this, beforeElement);
		}
	}

	void If::onUnmount() {
		m_thenComponent.tryUnmount();
		m_elseComponent.tryUnmount();
	}

	std::vector<const Component*> If::getChildren() const noexcept {
		return {m_condition.getValueOnce() ? m_thenComponent.get() : m_elseComponent.get()};
	}

	void If::updateCondition(bool b) {
		m_thenComponent.tryUnmount();
		m_elseComponent.tryUnmount();
		auto nextComp = getNextComponent();
		auto nextElement = nextComp ? nextComp->getFirstElement() : nullptr;
		if (b) {
			m_thenComponent.tryMount(this, nextElement);
		} else {
			m_elseComponent.tryMount(this, nextElement);
		}
	}


    void* ComponentParent::findContextProvider(const std::type_info&) noexcept {
        return nullptr;
    }

    void* InternalComponent::findContextProvider(const std::type_info& ti) noexcept {
        if (parent()) {
            return parent()->findContextProvider(ti);
        }
        return nullptr;
    }

	const ComponentParent* InternalComponent::toComponentParent() const noexcept {
		return this;
	}

    UseFont::UseFont(PropertyOrValue<const sf::Font*> pvf)
        : ContextProvider(std::move(pvf)) {
    }

	void List::onMount(const Element* beforeSibling) {
		for (auto& c : m_components) {
			c.tryMount(this, beforeSibling);
		}
	}

	void List::onUnmount() {
		for (auto it = m_components.rbegin(), itEnd = m_components.rend(); it != itEnd; ++it) {
			it->tryUnmount();
		}
	}

	std::vector<const Component*> List::getChildren() const noexcept {
		auto ret = std::vector<const Component*>();
		ret.reserve(m_components.size());
		for (const auto& c : m_components) {
			ret.push_back(c.get());
		}
		return ret;
	}

} // namespace ui
