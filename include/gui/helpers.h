#pragma once

#include "gui.h"
#include "stringhelpers.h"

namespace ui {

	struct LineBreak final : BlockElement {
		LineBreak(float height = 0.0f){
			setMargin(height * 0.5f);
			disable();
		}

		void render(sf::RenderWindow& rw) override {

		}
	};

	struct Button : Element {
		// TODO
	};
		
	struct PullDownMenu : Element {
		// TODO
	};

	struct Slider : Element {
		// TODO
	};

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