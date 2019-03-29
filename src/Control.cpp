#include "GUI/Control.hpp"

namespace ui {

    Control& Control::disable() noexcept {
		m_disabled = true;
		return *this;
	}

	Control& Control::enable() noexcept {
		m_disabled = false;
		return *this;
	}

	bool Control::isEnabled() const noexcept {
		return !m_disabled;
	}

	Control& Control::enableKeyboardNavigation() noexcept {
		m_keyboard_navigable = true;
		return *this;
	}

	Control& Control::disableKeyboardNavigation() noexcept {
		m_keyboard_navigable = false;
		return *this;
	}

	bool Control::keyboardNavigable() const noexcept {
		return m_keyboard_navigable;
	}

} // namespace ui