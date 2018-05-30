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

	struct CallbackButton : InlineElement {
		CallbackButton(std::string _label, sf::Font& font, std::function<void()> _callback = {})
			: label(add<Text>(_label, font)),
			callback(_callback),
			bgcolor(sf::Color(0xBBBBBBFF)) {

		}

		void setCallback(std::function<void()> _callback){
			callback = _callback;
		}

		void setCaption(std::string _label){
			label->setText(_label);
		}

		bool onLeftClick(int clicks) override {
			if (clicks == 1 && callback){
				callback();
			}
			bgcolor = sf::Color(0x888888FF);
			return true;
		}
		bool onLeftRelease() override {
			bgcolor = sf::Color(0xDDDDDDFF);
			return true;
		}

		bool onKeyDown(ui::Key key) override {
			if (key == ui::Key::Return && callback){
				callback();
			}
			return true;
		}

		void render(sf::RenderWindow& rw) override {
			sf::RectangleShape rect{getSize()};
			rect.setFillColor(bgcolor);
			rect.setOutlineThickness(1.0f);
			rect.setOutlineColor(sf::Color(0xFF));
			rw.draw(rect);
		}

		bool onMouseOver() override {
			bgcolor = sf::Color(0xDDDDDDFF);
			return false;
		}

		bool onMouseOut() override {
			bgcolor = sf::Color(0xBBBBBBFF);
			return false;
		}

	private:

		std::function<void()> callback;
		std::shared_ptr<Text> label;
		sf::Color bgcolor;
	};
		
	struct PullDownMenu : InlineElement {
		// TODO
	};

	struct Slider : InlineElement {
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