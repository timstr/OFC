#pragma once

#ifndef TIMSGUI_SCROLLPANEL_H
#define TIMSGUI_SCROLLPANEL_H

#include "GUI/Element.hpp"
// #include "GUI/GUI.hpp"

namespace ui {

	struct ScrollPanel : ui::BlockElement {
		ScrollPanel(bool horizontal_scrolling = true, bool vertical_scrolling = true, bool resize_button = false);

		Ref<Element> inner() const;

		void showHorizontalScrollBar();

		void hideHorizontalScrollBar();

		void showVerticalScrollBar();

		void hideVerticalScrollBar();

		void showResizeButton();

		void hideResizeButton();

		void setButtonNormalColor(sf::Color color);

		sf::Color getButtonNormalColor() const;

		void setButtonHoverColor(sf::Color color);

		sf::Color getButtonHoverColor() const;

		void setButtonThickness(float thickness);

		float getButtonThickness() const;

		void setButtonSpacing(float spacing);

		float getButtonSpacing() const;

		void setScrollSpeed(float speed);

		float getScrollSpeed() const;

	private:

		void onResize() override;

		void updatePositions();

		bool onScroll(float x, float y) override;

		bool onKeyDown(Key key) override;

		void moveInner(float x, float y);

		void gotoHome(bool horizontal);

		void gotoEnd(bool horizontal);

		struct ScrollButton : ui::FreeElement {
			ScrollButton();

			void onMouseOver() override;

			void onMouseOut() override;

			bool onLeftClick(int) override;

			void onLeftRelease() override;

			void fadeColor(sf::Color from, sf::Color to);

			static float thickness;
			static float spacing;
			static sf::Color hover_color;
			static sf::Color normal_color;
		};

		struct VerticalScrollBar : ScrollButton {
			VerticalScrollBar(ScrollPanel& _panel);

			void onDrag() override;

			void updateFromPanel();

			void movePanel();

			ScrollPanel& panel;
		};

		struct HorizontalScrollBar : ScrollButton {
			HorizontalScrollBar(ScrollPanel& _panel);

			void onDrag() override;

			void updateFromPanel();

			void movePanel();

			ScrollPanel& panel;
		};

		struct ResizeButton : ScrollButton {
			ResizeButton(ScrollPanel& _panel);

			void onDrag() override;

			ScrollPanel& panel;
		};

		struct InnerContent : ui::FreeElement {
			InnerContent(ScrollPanel& _panel);

			void onResize() override;

			ScrollPanel& panel;
		};

		Ref<FreeElement> clipping_container;
		Ref<InnerContent> innercontent;
		Ref<VerticalScrollBar> yscrollbar;
		Ref<HorizontalScrollBar> xscrollbar;
		Ref<ResizeButton> resizebtn;

		static float scrollspeed;
	};

} // namespace ui

#endif // TIMSGUI_SCROLLPANEL_H