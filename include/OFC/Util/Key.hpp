#pragma once

#include <SFML/Window/Keyboard.hpp>

#include <cstdint>

namespace ofc::ui {

    using Key = sf::Keyboard::Key;

    class ModifierKeys {
    public:
        bool alt() const noexcept;

        bool ctrl() const noexcept;

        bool shift() const noexcept;

        bool system() const noexcept;

    private:
        ModifierKeys(bool alt, bool ctrl, bool shift, bool system) noexcept;

        std::uint8_t m_values;

        friend class Window;
    };

} // namespace ofc::ui
