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
			state(State::Normal) {

			setBorderThickness(1.0f);
			setBorderColor(sf::Color(0xFF));
			setBackgroundColor(normal_color);
		}

		void setNormalColor(sf::Color color) {
			normal_color = color;
		}
		sf::Color getNormalColor() const {
			return normal_color;
		}
		void setHoverColor(sf::Color color) {
			hover_color = color;
		}
		sf::Color getHoverColor(sf::Color color) const {
			return hover_color;
		}
		void setActiveColor(sf::Color color) {
			active_color = color;
		}
		sf::Color getActiveColor() const {
			return active_color;
		}

		void setCallback(std::function<void()> _callback) {
			callback = _callback;
		}

		void setCaption(std::string _label) {
			label->setText(_label);
		}

		bool onLeftClick(int clicks) override {
			if (callback) {
				callback();
				fadeColor(active_color, hover_color, 0.15f);
			}
			return true;
		}
		void onLeftRelease() override {
			if (hovering()) {
				state = State::Hover;
			} else {
				fadeColor(getBackgroundColor(), normal_color, 0.15f);
				state = State::Normal;
			}

		}

		bool onKeyDown(ui::Key key) override {
			if (key == ui::Key::Return && callback) {
				callback();
				fadeColor(active_color, hovering() ? hover_color : normal_color, 0.15f);
			}
			return true;
		}

		void onMouseOver() override {
			if (state == State::Normal) {
				state = State::Hover;
				fadeColor(getBackgroundColor(), hover_color, 0.15f);
			}
		}
		void onMouseOut() override {
			if (state == State::Hover) {
				state = State::Normal;
				fadeColor(getBackgroundColor(), normal_color, 0.15f);
			}
		}

	private:

		std::function<void()> callback;
		std::shared_ptr<Text> label;
		sf::Color normal_color, hover_color, active_color;
		State state;

		void fadeColor(sf::Color from, sf::Color to, float seconds) {
			auto self = getThisAs<CallbackButton>();
			startTransition(seconds, [=](float t) {
				auto color = sf::Color(
					(uint8_t)(from.r * (1.0f - t) + to.r * t),
					(uint8_t)(from.g * (1.0f - t) + to.g * t),
					(uint8_t)(from.b * (1.0f - t) + to.b * t),
					255
				);
				setBackgroundColor(color);
			});
		}
	};


} // namespace ui