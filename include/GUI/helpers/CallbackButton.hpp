#pragma once

#ifndef TIMSGUI_CALLBACKBUTTON_H
#define TIMSGUI_CALLBACKBUTTON_H

#include "GUI/Element.hpp"
#include "GUI/Text.hpp"

namespace ui {

	struct CallbackButton : InlineElement {
	private:
		enum class State {
			Normal,
			Hover,
			Active
		};

	public:

		CallbackButton(std::string _label, const sf::Font& font, std::function<void()> _callback = {});

		void setNormalColor(sf::Color color);
		sf::Color getNormalColor() const;
		void setHoverColor(sf::Color color);
		sf::Color getHoverColor() const;
		void setActiveColor(sf::Color color);
		sf::Color getActiveColor() const;

		void setCallback(std::function<void()> _callback);

		void setCaption(std::string _label);

		bool onLeftClick(int) override;
		void onLeftRelease() override;

		bool onKeyDown(ui::Key key) override;

		void onMouseOver() override;
		void onMouseOut() override;

	private:

		std::function<void()> callback;
		Ref<Text> label;
		sf::Color normal_color, hover_color, active_color;
		State state;

		void fadeColor(sf::Color from, sf::Color to);
	};


} // namespace ui

#endif // TIMSGUI_CALLBACKBUTTON_H