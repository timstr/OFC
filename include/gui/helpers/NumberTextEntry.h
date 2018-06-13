#pragma once

#include "gui/textentry.h"

namespace ui {

	struct NumberTextEntry : TextEntry {
		NumberTextEntry(float default, float _min, float _max, std::function<void(float)> _callback, const sf::Font& font)
			: TextEntry(toString(default), font), value(default), min(_min), max(_max), callback(_callback) {
			setBackGroundColor(sf::Color(0xFFFFFFFF));
			setTextColor(sf::Color(0xFF));
		}

		void onReturn(std::string entered_text) override {
			if (validate(entered_text) && callback){
				float val = stringToFloat(entered_text);
				callback(val);
			}
		}

		void onType(std::string full_text) override {
			if (validate(full_text)){
				setBackGroundColor(sf::Color(0xFFFFFFFF));
			} else {
				setBackGroundColor(sf::Color(0xFF8080FF));
			}
		}

	private:
		float value;
		const float min;
		const float max;
		std::function<void(float)> callback;

		bool validate(const std::string& text){
			float val = stringToFloat(text);
			return !std::isnan(val);
		}
	};

} // namespace ui