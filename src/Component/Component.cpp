#include <GUI/Component/Component.hpp>

#include <GUI/Component/Text.hpp>

namespace ui {

    Component::Component() noexcept
        : m_isMounted(false)
        , m_parent(nullptr) {

    }

    Component::Component(Component&& c) noexcept 
        : ObserverOwner(std::move(static_cast<ObserverOwner&&>(c)))
        , m_isMounted(false)
        , m_parent(nullptr) {
        assert(!c.m_isMounted);
        assert(!c.m_parent);
    }

    Component::~Component() noexcept {
        // NOTE: unmounting here is not safe, since it would call a virtual function.
        // It may be assumed that all components will be unmounted before being destroyed.
        // Conversely, all components MUST unmount all their child components while
        // being unmounted.
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

    void Component::mount(ComponentParent* parent, const dom::Element* beforeSibling) {
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

    dom::Element* Component::getFirstElement() const noexcept {
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

    dom::Element* Component::getElement() const noexcept {
        return nullptr;
    }

    void Component::insertElement(std::unique_ptr<dom::Element> element, const dom::Element* beforeElement) {
        assert(m_parent);
        m_parent->onInsertChildElement(std::move(element), this, beforeElement);
    }

    void Component::eraseElement(dom::Element* element) {
        assert(m_parent);
        m_parent->onRemoveChildElement(element, this);
    }

    ComponentParent* Component::toComponentParent() noexcept {
        return const_cast<ComponentParent*>(const_cast<const Component*>(this)->toComponentParent());
    }

    const ComponentParent* Component::toComponentParent() const noexcept {
        return nullptr;
    }




    void ForwardingComponent::onInsertChildElement(std::unique_ptr<dom::Element> element, const Component* whichDescendent, const dom::Element* beforeElement) {
        assert(parent());
        parent()->onInsertChildElement(std::move(element), whichDescendent, beforeElement);
    }

    void ForwardingComponent::onRemoveChildElement(dom::Element* whichElement, const Component* whichDescendent) {
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

    AnyComponent::AnyComponent(const Property<String>& p)
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

    void AnyComponent::tryMount(InternalComponent* self, const dom::Element* beforeElement) {
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

} // namespace ui
