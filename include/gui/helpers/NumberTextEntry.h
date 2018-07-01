#pragma once

#include "gui/textentry.h"

namespace ui {

	struct NumberTextEntry : ui::InlineElement {
		NumberTextEntry(float defaultval, float _min, float _max, const sf::Font& font, std::function<void(float)> _callback){
			textfield = add<TextField>(defaultval, _min, _max, font, _callback);
		}

		bool onLeftClick(int clicks) override {
			textfield->startTyping();
			return true;
		}

		void setMinimum(float min){
			textfield->minimum = min;
		}
		float getMinimum() const {
			return textfield->minimum;
		}

		void setMaximum(float max){
			textfield->maximum = max;
		}
		float getMaximum(float max){
			return textfield->maximum;
		}

		void setValue(float val){
			textfield->value = std::min(std::max(textfield->minimum, val), textfield->maximum);
			textfield->setText(std::to_string(val));
		}
		float getValue() const {
			return textfield->value;
		}

	private:

		struct TextField : TextEntry {
			TextField(float defaultval, float _min, float _max, const sf::Font& font, std::function<void(float)> _callback)
				: TextEntry(toString(defaultval), font), value(defaultval), minimum(_min), maximum(_max), callback(_callback) {
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
				return (val >= minimum) && (val <= maximum);
			}
			std::function<void(float)> callback;

			float value;
			float minimum;
			float maximum;
		};

		std::shared_ptr<TextField> textfield;

	};

} // namespace ui