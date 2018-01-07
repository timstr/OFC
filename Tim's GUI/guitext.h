#pragma once

#include "guiwindow.h"

namespace ui {

	struct Text : Window {
		Text(const std::string& _text, const sf::Font& _font, sf::Color color = sf::Color(0xFF), int charsize = 15);

		void setText(const std::string& _text);
		std::string getText();
		void clearText();

		void setCharacterSize(unsigned int size);
		unsigned int getCharacterSize() const;

		void setTextColor(sf::Color color);
		const sf::Color& getTextColor() const;

		void setBackGroundColor(sf::Color color);
		const sf::Color& getBackGroundColor() const;

		void render(sf::RenderWindow& renderwin) override;

		protected:

		void updateSize();

		sf::Color background_color;
		sf::Text text;
	};

}