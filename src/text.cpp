#include "gui/text.h"

namespace ui {

	Text::Text(std::string _text, const sf::Font& _font, sf::Color color, unsigned charsize, TextStyle style)
		: Text(std::wstring { _text.begin(), _text.end() }, _font, color, charsize, style) {
	}

	Text::Text(std::wstring _text, const sf::Font& _font, sf::Color color, unsigned charsize, TextStyle style)
		: text(_text, _font) {
		text.setFillColor(color);
		setBorderColor(sf::Color(0));
		setBackgroundColor(sf::Color(0));
		setStyle(style);
		setMargin(0.0f);
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

	void Text::setCharacterSize(unsigned int size) {
		text.setCharacterSize(size);
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

	void Text::setStyle(TextStyle style) {
		text.setStyle(static_cast<uint32_t>(style));
	}

	TextStyle Text::getStyle() const {
		return static_cast<TextStyle>(text.getStyle());
	}

	void Text::render(sf::RenderWindow& rw) {
		Element::render(rw);
		rw.draw(text);
	}

	void Text::updateSize() {
		sf::FloatRect bounds = text.getGlobalBounds();
		text.setPosition({ ceil((float)getCharacterSize() / 5.0f), ceil((float)getCharacterSize() / 5.0f) });
		vec2 newsize;
		newsize.y = std::max((float)text.getCharacterSize(), bounds.height - bounds.top);
		if (text.getString().isEmpty()) {
			newsize.x = (float)text.getCharacterSize();
		} else {
			newsize.x = bounds.width + bounds.left;
		}
		setSize({
			ceil(newsize.x) + ceil((float)getCharacterSize() / 2.5f),
			ceil(newsize.y) + ceil((float)getCharacterSize() / 2.5f)
		}, true);
	}

}