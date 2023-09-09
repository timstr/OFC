#pragma once

#ifndef TIMSGUI_TEXTENTRYHELPER_H
#define TIMSGUI_TEXTENTRYHELPER_H

#include "GUI/TextEntry.hpp"

namespace ui {

	struct TextEntryHelper : InlineElement {
		TextEntryHelper(std::string str, const sf::Font& font,
			std::function<void(const std::wstring&)> on_return,
			std::function<bool(const std::wstring&)> validate = {},
			std::function<void(const std::wstring&)> on_change = {});

		bool onLeftClick(int) override;

		Ref<TextEntry> textEntry() const;

	private:

		struct TextField : TextEntry {
			TextField(std::string str, const sf::Font& font,
							std::function<void(const std::wstring&)> _on_return,
							std::function<bool(const std::wstring&)> _validate,
							std::function<void(const std::wstring&)> _on_change);

			void onReturn(std::wstring entered_text) override;

			void onType(std::wstring full_text) override;

			std::function<void(const std::wstring&)> on_return;
			std::function<bool(const std::wstring&)> validate;
			std::function<void(const std::wstring&)> on_change;
		};

		Ref<TextField> textfield;

	};

} // namespace ui


#endif // TIMSGUI_TEXTENTRYHELPER_H