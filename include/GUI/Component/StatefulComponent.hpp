#pragma once

#include <GUI/Component/Component.hpp>

namespace ui {

    template<typename StateType>
    class StatefulComponent : public ForwardingComponent {
    public:
        template<typename... Args>
        StatefulComponent(Args&&... args)
            : m_state(std::make_unique<StateType>(std::forward<Args>(args)...)) {
            
        }

    protected:
        virtual AnyComponent render() const = 0;

        StateType& stateMutable() const noexcept {
            assert(m_state != nullptr);
            return *m_state;
        }
        const StateType& state() const noexcept {
            assert(m_state != nullptr);
            return *m_state;
        }

    private:
        AnyComponent m_component;
        mutable std::unique_ptr<StateType> m_state;

        void onMount(const dom::Element* beforeElement) override final {
            m_component = render();
            m_component.tryMount(this, beforeElement);
        }

        void onUnmount() override final {
            m_component.tryUnmount();
        }

        std::vector<const Component*> getChildren() const noexcept override final {
            return { m_component.get() };
        }
    };

} // namespace ui
