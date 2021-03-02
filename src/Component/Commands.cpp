#include <OFC/Component/Commands.hpp>

namespace ofc::ui {
    
    UseCommands&& UseCommands::add(Key triggerKey, std::function<void()> callback) {
        m_commandData.emplace_back(CommandData{
            triggerKey,
            std::vector<ModifierKeys::KeyCode>{},
            std::move(callback),
            KeyboardCommand{}
        });
        return std::move(*this);
    }

    UseCommands&& UseCommands::add(Key triggerKey, ModifierKeys::KeyCode requiredKey, std::function<void()> callback) {
        m_commandData.emplace_back(CommandData{
            triggerKey,
            std::vector<ModifierKeys::KeyCode>{requiredKey},
            std::move(callback),
            KeyboardCommand{}
        });
        return std::move(*this);
    }

    UseCommands&& UseCommands::add(Key triggerKey, std::vector<ModifierKeys::KeyCode> requiredKeys, std::function<void()> callback) {
        m_commandData.emplace_back(CommandData{
            triggerKey,
            std::move(requiredKeys),
            std::move(callback),
            KeyboardCommand{}
        });
        return std::move(*this);
    }

    UseCommands&& UseCommands::with(AnyComponent c) {
        m_component = std::move(c);
        return std::move(*this);
    }

    void UseCommands::onMount(const dom::Element* beforeElement) {
        m_component.tryMount(this, beforeElement);
        if (const auto e = getFirstElement()) {
            if (const auto w = e->getParentWindow()) {
                for (auto& cd : m_commandData) {
                    cd.commandHandle = w->addKeyboardCommand(cd.triggerKey, cd.requiredKeys, cd.callback);
                }
            }
        }
    }

    void UseCommands::onUnmount() {
        if (const auto e = getFirstElement()) {
            if (const auto w = e->getParentWindow()) {
                for (auto& cd : m_commandData) {
                    cd.commandHandle.reset();
                }
            }
        }
        m_component.tryUnmount();
    }

    std::vector<const Component*> UseCommands::getPossibleChildren() const noexcept {
        return { m_component.get() };
    }

} // namespace ofc::ui
