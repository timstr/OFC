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
	private:
		enum class State {
			Normal,
			Hover,
			Active
		};

	public:

		CallbackButton(std::string _label, sf::Font& font, std::function<void()> _callback = {})
			: label(add<Text>(_label, font)),
			callback(_callback),
			normal_color(0xBBBBBBFF),
			hover_color(0xDDDDDDFF),
			active_color(0x888888FF),
			bgcolor(normal_color),
			state(State::Normal) {

		}

		void setNormalColor(sf::Color color){
			normal_color = color;
		}
		void setHoverColor(sf::Color color){
			hover_color = color;
		}
		void setActiveColor(sf::Color color){
			active_color = color;
		}

		void setCallback(std::function<void()> _callback){
			callback = _callback;
		}

		void setCaption(std::string _label){
			label->setText(_label);
		}

		bool onLeftClick(int clicks) override {
			if (callback){
				callback();
				fadeColor(active_color, hover_color, 0.15f);
			}
			return true;
		}
		bool onLeftRelease() override {
			if (hovering()){
				state = State::Hover;
			} else {
				fadeColor(bgcolor, normal_color, 0.15f);
				state = State::Normal;
			}

			return true;
		}

		bool onKeyDown(ui::Key key) override {
			if (key == ui::Key::Return && callback){
				callback();
				fadeColor(active_color, hovering() ? hover_color : normal_color, 0.15f);
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

		void onMouseOver() override {
			if (state == State::Normal){
				state = State::Hover;
				fadeColor(bgcolor, hover_color, 0.15f);
				
			}
		}
		void onMouseOut() override {
			if (state == State::Hover){
				state = State::Normal;
				fadeColor(bgcolor, normal_color, 0.15f);
			}
		}

	private:

		std::function<void()> callback;
		std::shared_ptr<Text> label;
		sf::Color bgcolor;
		sf::Color normal_color, hover_color, active_color;
		State state;

		void fadeColor(sf::Color from, sf::Color to, float seconds){
			auto self = getThisAs<CallbackButton>();
			startTransition(seconds, [=](float t){
				auto color = sf::Color(
					(uint8_t)(from.r * (1.0f - t) + to.r * t),
					(uint8_t)(from.g * (1.0f - t) + to.g * t),
					(uint8_t)(from.b * (1.0f - t) + to.b * t),
					255
				);
				self->bgcolor = color;
			});
		}
	};
		

	struct PullDownMenu : InlineElement {
		PullDownMenu(std::vector<std::string> options, sf::Font& font, std::function<void(std::string)> _onSelect)
			: collapsed(true), onSelect(_onSelect) {
			setMinSize({0.0f, 20.0f});

			caption = add<Text>(options.empty() ? "" : options.front(), font);
			
			list = add<FreeElement>();
			list->setPadding(0.0f);
			list->setVisible(false);
			for (const auto& option : options){
				list->add<ListItem>(option, font, [this,option]{
					this->caption->setText(option);
					if (this->onSelect){
						this->onSelect(option);
					}
					this->list->setVisible(false);
					this->grabFocus();
				});
			}
		}

		bool onLeftClick(int clicks) override {
			list->setVisible(true);
			return true;
		}

		void onLoseFocus() override {
			list->setVisible(false);
		}

		void render(sf::RenderWindow& rw) override {
			sf::RectangleShape rect {getSize()};
			rect.setFillColor(sf::Color(0xBBBBBBFF));
			rect.setOutlineColor(sf::Color(0xFF));
			rect.setOutlineThickness(1.0f);
			rw.draw(rect);
		}

	private:

		struct ListItem : BlockElement {
			ListItem(std::string _text, sf::Font& _font, std::function<void()> _callback) : callback(_callback), bgcolor(0xBBBBBBFF) {
				add<Text>(_text, _font);
				setMinSize({0.0f, 20.0f});
				setMargin(0.0f);
			}

			void onMouseOver() override {
				fadeColor(bgcolor, sf::Color(0xDDDDDDFF), 0.15f);
			}
			void onMouseOut() override {
				fadeColor(bgcolor, sf::Color(0xBBBBBBFF), 0.15f);
			}

			bool onLeftClick(int clicks) override {
				if (callback){
					callback();
				}
				return true;
			}

			void render(sf::RenderWindow& rw) override {
				sf::RectangleShape rect {getSize()};
				rect.setFillColor(bgcolor);
				rect.setOutlineColor(sf::Color(0xFF));
				rect.setOutlineThickness(1.0f);
				rw.draw(rect);
			}

			void fadeColor(sf::Color from, sf::Color to, float seconds){
				auto self = getThisAs<ListItem>();
				startTransition(seconds, [=](float t){
					auto color = sf::Color(
						(uint8_t)(from.r * (1.0f - t) + to.r * t),
						(uint8_t)(from.g * (1.0f - t) + to.g * t),
						(uint8_t)(from.b * (1.0f - t) + to.b * t),
						255
					);
					self->bgcolor = color;
				});
			}

			sf::Color bgcolor;
			const std::function<void()> callback;
		};

		bool collapsed;
		std::shared_ptr<Text> caption;
		std::shared_ptr<FreeElement> list;
		std::function<void(std::string)> onSelect;
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