#pragma once

#ifndef TIMSGUI_TEXTENTRY_H
#define TIMSGUI_TEXTENTRY_H

#include "Text.hpp"

namespace ui {

	struct TextEntry : Text {
		TextEntry(const sf::Font& font, int charsize = 15);
		TextEntry(std::wstring str, const sf::Font& font, int charsize = 15, sf::Color _text_color = sf::Color(0xFF), sf::Color _bg_color = sf::Color(0xFFFFFFFF));
		TextEntry(std::string str, const sf::Font& font, int charsize = 15, sf::Color _text_color = sf::Color(0xFF), sf::Color _bg_color = sf::Color(0xFFFFFFFF));

		void startTyping();
		void stopTyping();
		bool typing() const;
		void moveTo(vec2 pos);

		//to be overridden and used to deal with submission of newly typed text
		virtual void onReturn(std::wstring entered_text);
		virtual void onType(std::wstring full_text);

		void render(sf::RenderWindow& renderwindow) override;

		bool onKeyDown(Key key) override;
		bool onLeftClick(int clicks) override;
		void onFocus() override;
		void onLoseFocus() override;

	private:
		void write(char ch);
		void onBackspace();
		void onDelete();
		void onLeft();
		void onRight();
		void onHome();
		void onEnd();
		void updateCursorPosition() const;

		mutable size_t cursor_index = 0;
		mutable float cursor_pos;
		mutable float cursor_width;

		friend void run();
	};

}

#endif // TIMSGUI_TEXTENTRY_H