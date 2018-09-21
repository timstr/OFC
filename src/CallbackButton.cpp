#include "GUI/Helpers/CallbackButton.hpp"
#include "GUI/Text.hpp"
#include "GUI/GUI.hpp"

namespace ui {

	CallbackButton::CallbackButton(std::string _label, const sf::Font& font, std::function<void()> _callback)
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

	void CallbackButton::setNormalColor(sf::Color color) {
		normal_color = color;
		setBackgroundColor(normal_color);
	}
	sf::Color CallbackButton::getNormalColor() const {
		return normal_color;
	}
	void CallbackButton::setHoverColor(sf::Color color) {
		hover_color = color;
	}
	sf::Color CallbackButton::getHoverColor() const {
		return hover_color;
	}
	void CallbackButton::setActiveColor(sf::Color color) {
		active_color = color;
	}
	sf::Color CallbackButton::getActiveColor() const {
		return active_color;
	}

	void CallbackButton::setCallback(std::function<void()> _callback) {
		callback = _callback;
	}

	void CallbackButton::setCaption(std::string _label) {
		label->setText(_label);
	}

	bool CallbackButton::onLeftClick(int) {
		if (callback) {
			callback();
			fadeColor(active_color, hover_color);
		}
		return true;
	}
	void CallbackButton::onLeftRelease() {
		if (hovering()) {
			state = State::Hover;
		} else {
			fadeColor(backgroundColor(), normal_color);
			state = State::Normal;
		}

	}

	bool CallbackButton::onKeyDown(ui::Key key) {
		if ((key == ui::Key::Return || key == ui::Key::Space) && callback) {
			callback();
			fadeColor(active_color, hovering() ? hover_color : normal_color);
			return true;
		}
		return false;
	}

	void CallbackButton::onMouseOver() {
		if (state == State::Normal) {
			state = State::Hover;
			fadeColor(backgroundColor(), hover_color);
		}
	}
	void CallbackButton::onMouseOut() {
		if (state == State::Hover) {
			state = State::Normal;
			fadeColor(backgroundColor(), normal_color);
		}
	}

	void CallbackButton::fadeColor(sf::Color from, sf::Color to) {
		auto self = thisAs<CallbackButton>();
		startTransition(0.25f, [=](float t) {
			self->setBackgroundColor(sf::Color(
				(uint8_t)(from.r * (1.0f - t) + to.r * t),
				(uint8_t)(from.g * (1.0f - t) + to.g * t),
				(uint8_t)(from.b * (1.0f - t) + to.b * t),
				(uint8_t)(from.a * (1.0f - t) + to.a * t)
			));
		});
	}

} // namespace ui