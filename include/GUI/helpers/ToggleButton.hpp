#pragma once

#ifndef TIMSGUI_TOGGLEBUTTON_H
#define TIMSGUI_TOGGLEBUTTON_H

#include "GUI/Element.hpp"
#include "GUI/helpers/CallbackButton.hpp"

namespace ui {

	struct ToggleButton : CallbackButton {
		ToggleButton(bool _value, sf::Font& font, std::function<void(bool)> onChange, std::pair<std::string, std::string> labels = { "off", "on" })
			: CallbackButton(_value ? labels.second : labels.first, font, [this, labels, onChange] {
			this->value = !this->value;
			this->setCaption(this->value ? labels.second : labels.first);
			onChange(this->value);
		}), value(_value) {

		}

	private:

		bool value;
	};

} // namespace ui

#endif // TIMSGUI_TOGGLEBUTTON_H