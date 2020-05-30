#include <GUI/Component.hpp>

namespace ui {

	Component::Component() noexcept
		: m_isMounted(false)
		, m_parent(nullptr) {

	}

	Component::Component(Component&& c) noexcept 
		: m_isMounted(false)
		, m_parent(nullptr) {
		assert(!c.m_isMounted);
		assert(!c.m_parent);
	}

	Component::~Component() noexcept {
		// NOTE: unmounting here is not safe, since it would call a virtual function
		assert(!m_isMounted);
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



	void ComponentRoot::onInsertChildElement(std::unique_ptr<Element> element, const Component* whichDescendent, const Element* beforeElement) {
		auto c = element->toContainer();
		assert(c);

	}

	void ComponentRoot::onRemoveChildElement(Element* whichElement, const Component* whichDescendent) {
		a a a a a
	}



	void ForwardingComponent::onInsertChildElement(std::unique_ptr<Element> element, const Component* whichDescendent, const Element* beforeElement) {
		assert(parent());
		parent()->onInsertChildElement(std::move(element), whichDescendent, beforeElement);
	}

	void ForwardingComponent::onRemoveChildElement(Element* whichElement, const Component* whichDescendent) {
		assert(parent());
		parent()->onRemoveChildElement(whichElement, whichDescendent);
	}



	AnyComponent::AnyComponent() noexcept
		: m_component(nullptr) {

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

	Button& Button::caption(Property<String>& s) {
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
