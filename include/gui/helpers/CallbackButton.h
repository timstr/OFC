#pragma once

#include "gui/element.h"

namespace ui {

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


} // namespace ui