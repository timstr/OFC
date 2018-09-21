#include "GUI/Helpers/ToggleButton.hpp"

namespace ui {

	ToggleButton::ToggleButton(bool _value, const sf::Font& font, std::function<void(bool)> onChange, std::pair<std::string, std::string> labels) :
		CallbackButton(_value ? labels.second : labels.first, font, [this, labels, onChange] {
			this->value = !this->value;
			this->setCaption(this->value ? labels.second : labels.first);
			onChange(this->value);
		}),
		value(_value) {

	}

} // namespace ui