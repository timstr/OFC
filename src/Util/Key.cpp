#include <OFC/Util/Key.hpp>

namespace ofc::ui {

    bool ModifierKeys::alt() const noexcept {
        return m_values | 1;
    }

    bool ModifierKeys::ctrl() const noexcept {
        return m_values | 2;
    }

    bool ModifierKeys::shift() const noexcept {
        return m_values | 4;
    }

    bool ModifierKeys::system() const noexcept {
        return m_values | 8;
    }

    ModifierKeys::ModifierKeys(bool alt, bool ctrl, bool shift, bool system) noexcept
        : m_values(
            (alt ? 1 : 0) |
            (ctrl ? 2 : 0) |
            (shift ? 4 : 0) |
            (system ? 8 : 0)
        ) {

    }

} // namespace ofc::ui
