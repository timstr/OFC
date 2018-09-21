#pragma once

#ifndef TIMSGUI_SLIDER_H
#define TIMSGUI_SLIDER_H

#include "GUI/Element.hpp"
#include "GUI/Text.hpp"

namespace ui {

	struct Slider : InlineElement {
		Slider(float defaultval, float min, float max, const sf::Font& font, std::function<void(float)> _onChange);

		bool onLeftClick(int);

		void onLeftRelease();

		void setMinimum(float min);
		float getMinimum() const;

		void setMaximum(float max);
		float getMaximum();

		void setValue(float val);
		float getValue() const;

		Ref<Element> getHandle() const;

	private:

		void onResize();

		void moveHandleTo(float val);

		bool onKeyDown(ui::Key key);

		struct Handle : FreeElement {
			Handle(Slider& _slider);

			bool onLeftClick(int);

			void onLeftRelease();

			bool onRightClick(int);

			void onRightRelease();

			void onDrag();

			bool onKeyDown(ui::Key key);

			void updateFromPos();

			Slider& slider;
			float reference_position;
			bool fine_dragging;
			static const float fine_speed;
		};

		Ref<Handle> handle;
		Ref<Text> label;
		float minimum, maximum, value;
		std::function<void(float)> onChange;
	};

} // namespace ui

#endif // TIMSGUI_SLIDER_H