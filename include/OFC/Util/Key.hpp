#pragma once

#include <SFML/Window/Keyboard.hpp>

#include <vector>
#include <cstdint>

namespace ofc::ui {

    using Key = sf::Keyboard::Key;

    class ModifierKeys {
    public:
        bool alt() const noexcept;

        bool ctrl() const noexcept;

        bool shift() const noexcept;

        bool system() const noexcept;

        enum class KeyCode : std::uint8_t {
            Alt = 1,
            Ctrl = 2,
            Shift = 4,
            System = 8
        };
        
        bool hasAllKeysOf(const ModifierKeys& other) const noexcept;
        bool hasDifferentKeysFrom(const ModifierKeys& other) const noexcept;
        
        static constexpr auto Alt = KeyCode::Alt;
        static constexpr auto Ctrl = KeyCode::Ctrl;
        static constexpr auto Shift = KeyCode::Shift;
        static constexpr auto System = KeyCode::System;

    private:
        ModifierKeys() noexcept;

        ModifierKeys(bool alt, bool ctrl, bool shift, bool system) noexcept;

        ModifierKeys(const std::vector<KeyCode>& keyCodes) noexcept;

        std::uint8_t compile(bool alt, bool ctrl, bool shift, bool system) const noexcept;

        std::uint8_t compile(const std::vector<KeyCode>& keycodes) const noexcept;

        std::uint8_t m_values;

        friend class Window;
    };

} // namespace ofc::ui
