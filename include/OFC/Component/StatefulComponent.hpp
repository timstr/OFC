#pragma once

#include <OFC/Component/Component.hpp>

namespace ofc::ui {

    struct Persistent {};
    struct Ephemeral {};

    template<typename StateType, typename PersistenceType>
    class StatefulComponent : public ForwardingComponent {
    public:
        static_assert(
            std::is_same_v<PersistenceType, Persistent> || std::is_same_v<PersistenceType, Ephemeral>,
            "PersistenceType must be either ui::Persistent or ui::Ephemeral"
        );

        static_assert(std::is_class_v<StateType>, "StateType must be a class or struct");

        template<typename... Args>
        StatefulComponent(Args&&... args)
            : m_state(std::make_unique<StateType>(std::forward<Args>(args)...)) {
            
        }

    protected:
        virtual AnyComponent render() const = 0;

        StateType& stateMut() const noexcept {
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

        std::vector<const Component*> getPossibleChildren() const noexcept override final {
            return { m_component.get() };
        }

        void serializeStateImpl(Serializer& s) const override final {
            if constexpr (std::is_same_v<PersistenceType, Persistent>) {
                s.object<StateType>(state());
            }
            Component::serializeStateImpl(s);
        }

        void deserializeStateImpl(Deserializer& d) const override final {
            if constexpr (std::is_same_v<PersistenceType, Persistent>) {
                d.object<StateType>(stateMut());
            }
            Component::deserializeStateImpl(d);
        }
    };

} // namespace ofc::ui
