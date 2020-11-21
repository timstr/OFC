#include <OFC/Util/Key.hpp>

#include <cassert>

namespace ofc::ui {

    bool ModifierKeys::alt() const noexcept {
        return m_values & static_cast<std::uint8_t>(Alt);
    }

    bool ModifierKeys::ctrl() const noexcept {
        return m_values & static_cast<std::uint8_t>(Ctrl);
    }

    bool ModifierKeys::shift() const noexcept {
        return m_values & static_cast<std::uint8_t>(Shift);
    }

    bool ModifierKeys::system() const noexcept {
        return m_values & static_cast<std::uint8_t>(System);
    }

    bool ModifierKeys::hasAllKeysOf(const ModifierKeys& other) const noexcept {
        return (m_values & other.m_values) == other.m_values;
    }

    bool ModifierKeys::hasDifferentKeysFrom(const ModifierKeys& other) const noexcept {
        return m_values != other.m_values;
    }

    ModifierKeys::ModifierKeys() noexcept
        : m_values{0} {

    }
    
    ModifierKeys::ModifierKeys(bool alt, bool ctrl, bool shift, bool system) noexcept
        : m_values(compile(alt, ctrl, shift, system)) {

    }

    ModifierKeys::ModifierKeys(const std::vector<KeyCode>& keyCodes) noexcept
        : m_values(compile(keyCodes)) {
        
    }

    std::uint8_t ModifierKeys::compile(bool alt, bool ctrl, bool shift, bool system) const noexcept {
        return (
            (alt ? static_cast<std::uint8_t>(Alt) : 0) |
            (ctrl ? static_cast<std::uint8_t>(Ctrl) : 0) |
            (shift ? static_cast<std::uint8_t>(Shift) : 0) |
            (system ? static_cast<std::uint8_t>(System) : 0)
        );
    }

    std::uint8_t ModifierKeys::compile(const std::vector<KeyCode>& keyCodes) const noexcept {
        auto alt = false;
        auto ctrl = false;
        auto shift = false;
        auto system = false;
        for (const auto& c : keyCodes) {
            switch (c) {
            case Alt:
                alt = true;
                continue;
            case Ctrl:
                ctrl = true;
                continue;
            case Shift:
                shift = true;
                continue;
            case System:
                system = true;
                continue;
            default:
                assert(false);
            }
        }
        return compile(alt, ctrl, shift, system);
    }

} // namespace ofc::ui
