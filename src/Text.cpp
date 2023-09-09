#include "GUI/Text.hpp"

#include <cmath>

namespace ui {

	Text::Text(std::string _text, const sf::Font& _font, sf::Color color, unsigned charsize, uint32_t style)
		: Text(std::wstring { _text.begin(), _text.end() }, _font, color, charsize, style) {
	}

	Text::Text(std::wstring _text, const sf::Font& _font, sf::Color color, unsigned charsize, uint32_t style)
		: text(_text, _font) {
		text.setFillColor(color);
		setBorderColor(sf::Color(0));
		setBackgroundColor(sf::Color(0));
		setStyle(style);
		disable();
		setCharacterSize(charsize);
	}

	void Text::setText(std::string _text) {
		text.setString(_text);
		updateSize();
	}
	void Text::setText(std::wstring _text) {
		text.setString(_text);
		updateSize();
	}

	std::wstring Text::getText() {
		return text.getString();
	}

	void Text::clearText() {
		text.setString("");
		updateSize();
	}

	void Text::setCharacterSize(unsigned int char_size) {
		text.setCharacterSize(char_size);
		updateSize();
	}

	unsigned int Text::getCharacterSize() const {
		return text.getCharacterSize();
	}

	void Text::setTextColor(sf::Color color) {
		text.setFillColor(color);
	}

	const sf::Color& Text::getTextColor() const {
		return text.getFillColor();
	}

	void Text::setStyle(uint32_t style) {
		text.setStyle(style);
	}

	uint32_t Text::getStyle() const {
		return text.getStyle();
	}

	void Text::render(sf::RenderWindow& rw) {
		Element::render(rw);
		rw.draw(text);
	}

	void Text::updateSize() {
		sf::FloatRect bounds = text.getGlobalBounds();
		text.setPosition({ std::ceil((float)getCharacterSize() / 5.0f), std::ceil((float)getCharacterSize() / 5.0f) });
		vec2 newsize;
		newsize.y = std::max((float)text.getCharacterSize(), bounds.height - bounds.top);
		newsize.x = std::max((float)text.getCharacterSize(), bounds.width + bounds.left);
		setSize({
			std::ceil(newsize.x) + std::ceil((float)getCharacterSize() / 2.5f),
			std::ceil(newsize.y) + std::ceil((float)getCharacterSize() / 2.5f)
		}, true);
	}

}