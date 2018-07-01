#pragma once

#include "gui/textentry.h"

namespace ui {

	// TODO: allow limits and value to be changed

	struct NumberTextEntry : ui::InlineElement {
		NumberTextEntry(float defaultval, float _min, float _max, const sf::Font& font, std::function<void(float)> _callback){
			textfield = add<TextField>(defaultval, _min, _max, font, _callback);
		}

		bool onLeftClick(int clicks) override {
			textfield->beginTyping();
			return true;
		}

	private:

		struct TextField : TextEntry {
			TextField(float defaultval, float _min, float _max, const sf::Font& font, std::function<void(float)> _callback)
				: TextEntry(toString(defaultval), font), value(defaultval), min(_min), max(_max), callback(_callback) {
				setBackgroundColor(sf::Color(0xFFFFFFFF));
				setTextColor(sf::Color(0xFF));
			}

			void onReturn(std::wstring entered_text) override {
				if (validate(entered_text) && callback){
					float val = stringToFloat(entered_text);
					callback(val);
				}
			}

			void onType(std::wstring full_text) override {
				if (validate(full_text)){
					setBackgroundColor(sf::Color(0xFFFFFFFF));
				} else {
					setBackgroundColor(sf::Color(0xFF8080FF));
				}
			}

			bool validate(const std::wstring& text){
				float val = stringToFloat(text);
				if (std::isnan(val)){
					return false;
				}
				return (val >= min) && (val <= max);
			}
			std::function<void(float)> callback;

			float value;
			const float min;
			const float max;
		};

		std::shared_ptr<TextField> textfield;

	};

} // namespace ui