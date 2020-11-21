#pragma once

#include <OFC/Component/Component.hpp>
#include <OFC/Util/Key.hpp>
#include <OFC/Window.hpp>

namespace ofc::ui {

    class UseCommands : public ForwardingComponent {
    public:
        UseCommands() = default;

        UseCommands& add(Key triggerKey, std::function<void()> callback);

        UseCommands& add(Key triggerKey, ModifierKeys::KeyCode requiredKey, std::function<void()> callback);

        UseCommands& add(Key triggerKey, std::vector<ModifierKeys::KeyCode> requiredKeys, std::function<void()> callback);

        UseCommands& with(AnyComponent);

    private:
        struct CommandData {
            Key triggerKey;
            std::vector<ModifierKeys::KeyCode> requiredKeys;
            std::function<void()> callback;
            KeyboardCommand commandHandle;
        };
        std::vector<CommandData> m_commandData;
        AnyComponent m_component;

        void onMount(const dom::Element* beforeElement) override final;

        void onUnmount() override final;

        std::vector<const Component*> getPossibleChildren() const noexcept override final;
    };

} // namespace ofc
