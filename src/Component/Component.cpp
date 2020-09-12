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

    bool Component::appearsBefore(const Component* other) const noexcept {
        if (this == other) {
            return false;
        }

        const auto walkPath = [](const Component* comp) {
            std::vector<const Component*> path;
            while (true) {
                path.push_back(comp);
                if (auto p = comp->parent()) {
                    if (auto c = p->toComponent()) {
                        comp = c;
                        continue;
                    }
                }
                break;
            }
            return path;
        };

        const auto pathToSelf = walkPath(this);
        const auto pathToOther = walkPath(other);

        auto itSelf = pathToSelf.rbegin();
        auto itOther = pathToOther.rbegin();

        auto parent = (*itSelf)->parent();

        if (parent != (*itOther)->parent()) {
            return false;
        }

        while (itSelf != pathToSelf.rend() && itOther != pathToOther.rend()) {
            if (*itSelf != *itOther) {
                for (const auto& c : parent->getChildren()) {
                    if (c == *itSelf) {
                        return true;
                    } else if (c == *itOther) {
                        return false;
                    }
                }
                assert(false);
            }
            ++itSelf;
            ++itOther;
            parent = (*itSelf)->parent();
        }
        assert(false);
        return false;
    }

    dom::Element* Component::getElement() const noexcept {
        return nullptr;
    }

    void Component::serializeState(Serializer& s) const {
        serializeStateImpl(s);
    }

    void Component::deserializeState(Deserializer& d) {
        deserializeStateImpl(d);
    }

    void Component::insertElement(std::unique_ptr<dom::Element> element, const dom::Element* beforeElement) {
        assert(m_parent);
        m_parent->onInsertChildElement(std::move(element), Scope{this, beforeElement});
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

    void Component::serializeStateImpl(Serializer& s) const {
        if (auto cp = toComponentParent()) {
            for (auto c : cp->getChildren()) {
                c->serializeStateImpl(s);
            }
        }
    }

    void Component::deserializeStateImpl(Deserializer& d) const {
        if (auto cp = toComponentParent()) {
            for (auto c : cp->getChildren()) {
                c->deserializeStateImpl(d);
            }
        }
    }




    void ForwardingComponent::onInsertChildElement(std::unique_ptr<dom::Element> element, const Scope& scope) {
        assert(parent());
        parent()->onInsertChildElement(std::move(element), scope);
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
            if (beforeElement == nullptr) {
                auto nextComp = self->getNextComponent();
                beforeElement = nextComp ? nextComp->getFirstElement() : nullptr;
            }
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

    void AnyComponent::reset() {
        assert(!isMounted());
        m_component = nullptr;
    }

    

    void* ComponentParent::findContextProvider(const std::type_info&) noexcept {
        return nullptr;
    }

    Component* ComponentParent::toComponent() noexcept {
        return const_cast<Component*>(const_cast<const ComponentParent*>(this)->toComponent());
    }

    const Component* ComponentParent::toComponent() const noexcept {
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

    const Component* InternalComponent::toComponent() const noexcept {
        return this;
    }


    Restorable::Context::Context() noexcept
        : m_restorable(nullptr) {
    }

    Restorable::Context::~Context() noexcept {
        if (m_restorable) {
            assert(m_restorable->m_context == this);
            m_restorable->m_context = nullptr;
        }
    }

    Restorable::Context::Context(Context&& o) noexcept
        : m_restorable(std::exchange(o.m_restorable, nullptr)) {

        if (m_restorable) {
            assert(m_restorable->m_context == &o);
            m_restorable->m_context = this;
        }
    }

    Restorable::Context& Restorable::Context::operator=(Context&& o) noexcept {
        if (m_restorable) {
            assert(m_restorable->m_context == this);
            m_restorable->m_context = nullptr;
        }
        m_restorable = std::exchange(o.m_restorable, nullptr);
        if (m_restorable) {
            assert(m_restorable->m_context == &o);
            m_restorable->m_context = this;
        }
        return *this;
    }

    void Restorable::Context::save(Serializer& s) const {
        assert(m_restorable);
        auto c = m_restorable->m_component.get();
        assert(c);
        assert(c->isMounted());
        c->serializeState(s);
    }

    void Restorable::Context::restore(Deserializer& d) {
        assert(m_restorable);
        auto c = m_restorable->m_component.get();
        assert(c);
        assert(c->isMounted());
        c->deserializeState(d);
    }

    Restorable::Restorable(Context& c)
        : m_context(&c) {
    }

    Restorable::~Restorable() {
        if (m_context) {
            m_context->m_restorable = nullptr;
        }
    }

    Restorable::Restorable(Restorable&& o)
        : m_component(std::move(o.m_component))
        , m_context(std::exchange(o.m_context, nullptr)) {

        if (m_context) {
            assert(m_context->m_restorable == &o);
            m_context->m_restorable = this;
        }
    }

    Restorable& Restorable::with(AnyComponent c) {
        m_component = std::move(c);
        return *this;
    }

    void Restorable::onMount(const dom::Element* beforeSibling) {
        m_component.tryMount(this, beforeSibling);
    }

    void Restorable::onUnmount() {
        m_component.tryUnmount();
    }

    Scope::Scope(const Component* descendent, const dom::Element* beforeElement) noexcept
        : m_descendent(descendent)
        , m_beforeElement(beforeElement) {
    }

    const Component* Scope::descendent() const noexcept {
        return m_descendent;
    }

    const dom::Element* Scope::beforeElement() const noexcept {
        return m_beforeElement;
    }

    SimpleForwardingComponent::SimpleForwardingComponent(AnyComponent c)
        : m_child(std::move(c)) {

    }

    void SimpleForwardingComponent::onMount(const dom::Element* e) {
        m_child.tryMount(this, e);
    }

    void SimpleForwardingComponent::onUnmount() {
        m_child.tryUnmount();
    }

    std::vector<const Component*> SimpleForwardingComponent::getChildren() const noexcept {
        return { m_child.get() };
    }

} // namespace ui
