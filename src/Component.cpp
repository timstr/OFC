#include <GUI/Component.hpp>

namespace ui {

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

	void Component::mount(ComponentParent* parent) {
		assert(!m_isMounted);
		assert(!m_parent);
		m_parent = parent;
		m_isMounted = true;
		onMount();
	}

	void Component::unmount() {
		assert(m_isMounted);
		assert(m_parent);
		onUnmount();
		m_parent = nullptr;
		m_isMounted = false;
	}

	void Component::insertElement(std::unique_ptr<Element> element, const Element* beforeElement) {
		assert(m_parent);
		m_parent->onInsertChildElement(std::move(element), this, beforeElement);
	}

	void Component::eraseElement(Element* element) {
		assert(m_parent);
		m_parent->onRemoveChildElement(element, this);
	}



    std::unique_ptr<Container> ComponentRoot::mount() {
        assert(m_component);
        assert(!m_tempContainer);
        m_component->mount(this);
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
        assert(!m_component->isMounted());
	}

    AnyComponent& AnyComponent::operator=(AnyComponent&& ac) noexcept {
        if (&ac == this) {
            return *this;
        }
        assert(!m_component || !m_component->isMounted());
        assert(!ac.m_component || !ac.m_component->isMounted());
        m_component = std::exchange(ac.m_component, nullptr);
        return *this;
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

	void AnyComponent::tryMount(InternalComponent* self) {
		if (m_component && !m_component->isMounted()){
			m_component->mount(self);
		}
	}

	void AnyComponent::tryUnmount() {
		if (m_component && m_component->isMounted()){
			m_component->unmount();
		}
	}



	StaticText::StaticText(const sf::Font& f, sf::String s)
		: m_font(&f)
		, m_string(std::move(s)) {

	}

	std::unique_ptr<Text> StaticText::createElement() {
		return std::make_unique<Text>(m_string, *m_font, sf::Color::Black);
	}

	DynamicText::DynamicText(const sf::Font& f, Property<String>& s)
		: m_font(&f)
		, m_stringObserver(this, &DynamicText::updateString, s) {

	}

	std::unique_ptr<Text> DynamicText::createElement() {
		return std::make_unique<Text>(
			m_stringObserver.getValueOnce(),
			*m_font,
			sf::Color::Black
		);
	}

	void DynamicText::updateString(const String& s) {
		element()->setText(s);
	}



	Button::Button(const ui::Font& f)
		: m_font(&f)
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
			*m_font,
			m_onClick
		);
	}

	void Button::updateCaption(const String& s) {
		element()->getCaption().setText(s);
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
	void If::onMount() {
		if (m_condition.getValueOnce()) {
			m_thenComponent.tryMount(this);
		} else {
			m_elseComponent.tryMount(this);
		}
	}

	void If::onUnmount() {
		m_thenComponent.tryUnmount();
		m_elseComponent.tryUnmount();
	}

	void If::updateCondition(const bool& b) {
		m_thenComponent.tryUnmount();
		m_elseComponent.tryUnmount();
		if (b) {
			m_thenComponent.tryMount(this);
		} else {
			m_elseComponent.tryMount(this);
		}
	}


} // namespace ui
