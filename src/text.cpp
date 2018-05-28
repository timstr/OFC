#include "gui/text.h"

namespace ui {

	Text::Text(std::string _text, const sf::Font& _font, sf::Color color, int charsize)
		: Text(std::wstring{_text.begin(), _text.end()}, _font, color, charsize) {
	}
	
	Text::Text(std::wstring _text, const sf::Font& _font, sf::Color color, int charsize)
		: text(_text, _font, charsize) {
		text.setFillColor(color);
		updateSize();
		disable();
		setMargin((float)charsize / 7.5f);
		background_color = sf::Color(0x0);
	}
	
	void Text::setText(std::string _text){
		text.setString(_text);
		updateSize();
	}
	
	std::string Text::getText(){
		return text.getString().toAnsiString();
	}
	
	void Text::clearText(){
		text.setString("");
		updateSize();
	}
	
	void Text::setCharacterSize(unsigned int size){
		text.setCharacterSize(size);
		updateSize();
	}
	
	unsigned int Text::getCharacterSize() const {
		return text.getCharacterSize();
	}
	
	void Text::setTextColor(sf::Color color){
		text.setFillColor(color);
	}
	
	const sf::Color& Text::getTextColor() const {
		return text.getFillColor();
	}
	
	void Text::setBackGroundColor(sf::Color color){
		background_color = color;
	}
	
	const sf::Color& Text::getBackGroundColor() const {
		return background_color;
	}
	
	void Text::render(sf::RenderWindow& rw){
		sf::RectangleShape rect(getSize());
		rect.setFillColor(background_color);
		rw.draw(rect);
		rw.draw(text);
	}
	
	void Text::updateSize(){
		sf::FloatRect bounds = text.getGlobalBounds();
		vec2 newsize;
		newsize.y = std::max((float)text.getCharacterSize(), bounds.height - bounds.top);
		if (text.getString().isEmpty()){
			newsize.x = (float)text.getCharacterSize();
		} else {
			newsize.x = bounds.width + bounds.left;
		}
		setMinSize(newsize);
	}

}