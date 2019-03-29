#include "GUI/Helpers/TextEntryHelper.hpp"

namespace ui {

	TextEntryHelper::TextEntryHelper(std::string str, const sf::Font& font,
		std::function<void(const std::wstring&)> on_return,
		std::function<bool(const std::wstring&)> validate,
		std::function<void(const std::wstring&)> on_change) {

		textfield = add<TextField>(str, font, on_return, validate, on_change);
	}

	bool TextEntryHelper::onLeftClick(int) {
		textfield->startTyping();
		return true;
	}

	ui::StrongRef<TextEntry> TextEntryHelper::textEntry() const {
		return textfield;
	}


	TextEntryHelper::TextField::TextField(std::string str, const sf::Font& font,
		std::function<void(const std::wstring&)> _on_return,
		std::function<bool(const std::wstring&)> _validate,
		std::function<void(const std::wstring&)> _on_change)
		: TextEntry(str, font), on_return(_on_return), validate(_validate), on_change(_on_change) {

		setBackgroundColor(sf::Color(0xFFFFFFFF));
		setTextColor(sf::Color(0xFF));
	}

	void TextEntryHelper::TextField::onReturn(std::wstring entered_text) {
		if (on_return && (!validate || validate(entered_text))) {
			on_return(entered_text);
		}
	}

	void TextEntryHelper::TextField::onType(std::wstring full_text) {
		if (validate) {
			if (validate(full_text)) {
				setBackgroundColor(sf::Color(0xFFFFFFFF));
			} else {
				setBackgroundColor(sf::Color(0xFF8080FF));
			}
		}
		if (on_change) {
			on_change(full_text);
		}
	}

} // namespace ui