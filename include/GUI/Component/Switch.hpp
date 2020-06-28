#pragma once

#include <GUI/Component/Component.hpp>

#include <map>

namespace ui {

    template<typename T>
    class Switch : public ForwardingComponent {
    public:
        Switch(PropertyOrValue<T> p)
            : m_observer(this, &Switch<T>::handleUpdate, std::move(p)) {

        }

        Switch& Case(T t, AnyComponent c) {
            m_cases.insert_or_assign(std::move(t), std::move(c));
            return *this;
        }

        Switch& Default(AnyComponent c) {
            m_defaultComponent = std::move(c);
            return *this;
        }

    private:
        Observer<T> m_observer;
        std::map<T, AnyComponent> m_cases;
        AnyComponent m_defaultComponent;

        void onMount(const dom::Element* beforeSibling) override final {
            const auto& v = m_observer.getValueOnce();
            if (auto it = m_cases.find(v); it != m_cases.end()) {
                it->second.tryMount(this, beforeSibling);
            } else {
                m_defaultComponent.tryMount(this, beforeSibling);
            }
        }

        void onUnmount() override final {
            for (auto& it : m_cases) {
                it.second.tryUnmount();
            }
            m_defaultComponent.tryUnmount();
        }

        std::vector<const Component*> getChildren() const noexcept override final {
            auto ret = std::vector<const Component*>{};
            for (auto& it : m_cases) {
                const auto& c = it.second;
                if (c.isMounted()){
                    ret.push_back(c.get());
                };
            }
            if (m_defaultComponent.isMounted()) {
                ret.push_back(m_defaultComponent.get());
            }
            assert(ret.size() <= 1);
            return ret;
        }

        void handleUpdate(DiffArgType<T> d) {
            for (auto& it : m_cases) {
                it.second.tryUnmount();
            }
            m_defaultComponent.tryUnmount();

            if (auto it = m_cases.find(d); it != m_cases.end()) {
                it->second.tryMount(this, nullptr);
            } else {
                m_defaultComponent.tryMount(this, nullptr);
            }
        }
    };



    template<typename T>
    Switch(const Property<T>&) -> Switch<T>;

    template<typename T, typename U, typename... Rest>
    Switch(DerivedProperty<T, U, Rest...>) -> Switch<T>;

} // namespace ui
