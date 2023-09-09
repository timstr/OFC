#pragma once

#ifndef TIMSGUI_NUMBERTEXTENTRY_H
#define TIMSGUI_NUMBERTEXTENTRY_H

#include "GUI/TextEntry.hpp"

namespace ui {

	template<typename NumberType>
	struct NumberTextEntry : ui::InlineElement {
		NumberTextEntry(NumberType defaultval, NumberType _min, NumberType _max, const sf::Font& font, std::function<void(NumberType)> _callback) {
			static_assert(std::is_arithmetic_v<NumberType>, "The NumberType for NumberTextEntry must be a number");
			textfield = add<TextField>(defaultval, _min, _max, font, _callback);
		}

		bool onLeftClick(int) override {
			textfield->startTyping();
			return true;
		}

		void setMinimum(NumberType min) {
			textfield->minimum = min;
		}
		NumberType getMinimum() const {
			return textfield->minimum;
		}

		void setMaximum(NumberType max) {
			textfield->maximum = max;
		}
		NumberType getMaximum(NumberType max) {
			return textfield->maximum;
		}

		void setValue(NumberType val) {
			textfield->value = std::min(std::max(textfield->minimum, val), textfield->maximum);
			textfield->setText(std::to_string(val));
		}
		NumberType getValue() const {
			return textfield->value;
		}

	private:

		struct TextField : TextEntry {
			TextField(NumberType defaultval, NumberType _min, NumberType _max, const sf::Font& font, std::function<void(NumberType)> _callback)
				: TextEntry(toString(defaultval), font), value(defaultval), minimum(_min), maximum(_max), callback(_callback) {
				setBackgroundColor(sf::Color(0xFFFFFFFF));
				setTextColor(sf::Color(0xFF));
			}

			void onReturn(std::wstring entered_text) override {
				if (validate(entered_text) && callback) {
					NumberType val = stringTo<NumberType>(entered_text);
					callback(val);
				}
			}

			void onType(std::wstring full_text) override {
				if (validate(full_text)) {
					setBackgroundColor(sf::Color(0xFFFFFFFF));
				} else {
					setBackgroundColor(sf::Color(0xFF8080FF));
				}
			}

			bool validate(const std::wstring& entered_text) {
				auto val = stringTo<NumberType>(entered_text);
				if (!val) {
					return false;
				}
				return (val.getValue() >= minimum) && (val.getValue() <= maximum);
			}
			std::function<void(NumberType)> callback;

			NumberType value;
			NumberType minimum;
			NumberType maximum;
		};

		Ref<TextField> textfield;

	};

} // namespace ui

#endif // TIMSGUI_NUMBERTEXTENTRY_H