#pragma once

#include "gui/textentry.h"

namespace ui {

	struct TextEntryHelper : TextEntry {
		TextEntryHelper(std::string str, const sf::Font& font,
						std::function<void(const std::string&)> _on_return,
						std::function<bool(const std::string&)> _validate = {},
						std::function<void(const std::string&)> _on_change = {})
			: TextEntry(str, font), on_return(_on_return), validate(_validate), on_change(_on_change) {

			setBackGroundColor(sf::Color(0xFFFFFFFF));
			setTextColor(sf::Color(0xFF));
		}

		void onReturn(std::string entered_text) override {
			if (on_return && (!validate || validate(entered_text))){
				on_return(entered_text);
			}
		}

		void onType(std::string full_text) override {
			if (validate){
				if (validate(full_text)){
					setBackGroundColor(sf::Color(0xFFFFFFFF));
				} else {
					setBackGroundColor(sf::Color(0xFF8080FF));
				}
			}
			if (on_change){
				on_change(full_text);
			}
		}

	private:
		std::function<void(const std::string&)> on_return;
		std::function<bool(const std::string&)> validate;
		std::function<void(const std::string&)> on_change;
	};

} // namespace ui