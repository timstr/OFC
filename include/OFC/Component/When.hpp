#pragma once

#include <OFC/Component/Component.hpp>

#include <optional>

namespace ofc::ui {

    /**
     * `When` is a component for safely dereferencing a pointer. Similar to `If`, it only mounts
     * its child component when the provided property is a non-null pointer. Unlike `If`, the
     * component is provided by a function taking the dereferenced pointer. The child component
     * is regenerated every time the pointer is assigned a new non-null pointee. This allows
     * the child component to safely hold a reference to the pointee and assume it will remain
     * valid.
     * Note that `When` is not aware of any state changes in the value that is pointed to, only
     * changes to the pointer's address. To subscribe to changes in the pointee itself, the
     * pointee should contain some kind of Value from which more components can be generated.
     */
    template<typename T>
    class When : public ForwardingComponent {
    public:
        When(Valuelike<T*> pv)
            : m_value(this, &When<T>::updateValue, std::move(pv)) {

        }

        When& Do(std::function<AnyComponent(CRefOrValue<T>)> f) {
            assert(!m_fn);
            m_fn = std::move(f);
            return *this;
        }

        When& Otherwise(AnyComponent c) {
            m_otherwiseComponent = std::move(c);
            return *this;
        }

    private:
        Observer<T*> m_value;
        std::function<AnyComponent(CRefOrValue<T>)> m_fn;
        AnyComponent m_doComponent;
        AnyComponent m_otherwiseComponent;

        void onMount(const dom::Element* beforeSibling) override final {
            assert(m_fn);
            if (auto p = m_value.getValueOnce()) {
                m_doComponent = m_fn(*p);
                m_doComponent.tryMount(this, beforeSibling);
            } else {
                m_otherwiseComponent.tryMount(this, beforeSibling);
            }
        }

        void onUnmount() override final {
            m_doComponent.tryUnmount();
            m_otherwiseComponent.tryUnmount();
        }

        std::vector<const Component*> getChildren() const noexcept override final {
            assert(!m_doComponent.isMounted() || !m_otherwiseComponent.isMounted());
            if (m_doComponent) {
                return { m_doComponent.get() };
            } else if (m_otherwiseComponent) {
                return { m_otherwiseComponent.get() };
            } else {
                return {};
            }
        }

        void updateValue(T* p) {
            assert(m_fn);
            m_doComponent.tryUnmount();
            m_otherwiseComponent.tryUnmount();
            if (p) {
                m_doComponent = m_fn(*p);
                m_doComponent.tryMount(this, nullptr);
            } else {
                m_otherwiseComponent.tryMount(this, nullptr);
            }
        }
    };

    template<typename T>
    When(const Value<T*>&) -> When<T>;

    template<typename T, typename U, typename... Rest>
    When(DerivedValue<T*, U, Rest...>) -> When<T>;

} // namespace ofc::ui 
