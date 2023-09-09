#pragma once

#ifndef TIMSGUI_TOGGLEBUTTON_H
#define TIMSGUI_TOGGLEBUTTON_H

#include "GUI/Element.hpp"
#include "GUI/Helpers/CallbackButton.hpp"

namespace ui {

	struct ToggleButton : CallbackButton {
		ToggleButton(bool _value, const sf::Font& font, std::function<void(bool)> onChange, std::pair<std::string, std::string> labels = { "off", "on" });

	private:

		bool value;
	};

} // namespace ui

#endif // TIMSGUI_TOGGLEBUTTON_H