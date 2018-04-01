#pragma once

#include "text.h"

namespace ui {

	// TODO: pressing escape shouldn't type a blank character
	struct TextEntry : Text {
		TextEntry(const sf::Font& font, int charsize = 15);
		TextEntry(std::string str, const sf::Font& font, int charsize = 15, sf::Color _text_color = sf::Color(0xFF), sf::Color _bg_color = sf::Color(0xFFFFFFFF));

		void beginTyping();
		void endTyping();
		void moveTo(vec2 pos);

		//to be overridden and used to deal with submission of newly typed text
		virtual void onReturn(std::string entered_text);
		virtual void onType(std::string full_text);

		void render(sf::RenderWindow& renderwindow) override;

		void onLeftClick(int clicks) override;
		void onFocus() override;

		private:
		void write(char ch);
		void onBackspace();
		void onDelete();
		void onLeft();
		void onRight();
		void onHome();
		void onEnd();
		void positionCursor();

		size_t cursor_index = 0;
		float cursor_pos;
		float cursor_width;

		friend void run();
	};

}