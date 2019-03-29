#pragma once

#ifndef TIMSGUI_TEXT_H
#define TIMSGUI_TEXT_H

#include "GUI/Element.hpp"

namespace ui {

	struct Text : final Element {

		Text(std::string _text, const sf::Font& _font, sf::Color color = sf::Color(0xFF), unsigned charsize = 15u, uint32_t style = TextStyle::Regular);
		Text(std::wstring _text, const sf::Font& _font, sf::Color color = sf::Color(0xFF), unsigned charsize = 15u, uint32_t style = TextStyle::Regular);

		void setText(std::string _text);
		void setText(std::wstring _text);
		std::wstring getText();
		void clearText();

		void setCharacterSize(unsigned int size);
		unsigned int getCharacterSize() const;

		void setTextColor(sf::Color color);
		const sf::Color& getTextColor() const;

		void setStyle(uint32_t style);
		uint32_t getStyle() const;

		void render(sf::RenderWindow& renderwin) override;

	protected:

		void updateSize();

		sf::Text text;
	};

}

#endif // TIMSGUI_TEXT_H