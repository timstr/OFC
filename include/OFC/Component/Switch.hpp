#pragma once

#include <OFC/Component/Component.hpp>

#include <map>

namespace ofc::ui {

    template<typename T>
    class Switch : public ForwardingComponent {
    public:
        Switch(Value<T> p)
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
            const auto& v = m_observer.getOnce();
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

        std::vector<const Component*> getPossibleChildren() const noexcept override final {
            auto ret = std::vector<const Component*>{};
            ret.reserve(m_cases.size() + 1);
            for (auto& it : m_cases) {
                ret.push_back(it.second.get());
            }
            ret.push_back(m_defaultComponent.get());
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
    Switch(const Value<T>&) -> Switch<T>;

} // namespace ofc::ui
