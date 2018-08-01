#pragma once

#include "GUI/Element.hpp"
#include "GUI/GUI.hpp"

namespace ui {

	struct ScrollPanel : ui::InlineElement {
		ScrollPanel(bool horizontal_scrolling = true, bool vertical_scrolling = true, bool resize_button = false) {
			clipping_container = add<FreeElement>();
			clipping_container->setClipping(true);
			innercontent = clipping_container->add<InnerContent>(*this);
			xscrollbar = add<HorizontalScrollBar>(*this);
			yscrollbar = add<VerticalScrollBar>(*this);
			resizebtn = add<ResizeButton>(*this);
			if (!horizontal_scrolling) {
				release(xscrollbar);
			}
			if (!vertical_scrolling) {
				release(yscrollbar);
			}
			if (!resizebtn) {
				release(resizebtn);
			}
		}

		Ref<Element> inner() const {
			return innercontent;
		}

		void showHorizontalScrollBar() {
			adopt(xscrollbar);
		}

		void hideHorizontalScrollBar() {
			release(xscrollbar);
		}

		void showVerticalScrollBar() {
			adopt(yscrollbar);
		}

		void hideVerticalScrollBar() {
			release(yscrollbar);
		}

		void showResizeButton() {
			adopt(resizebtn);
		}

		void hideResizeButton() {
			release(resizebtn);
		}

		void setButtonNormalColor(sf::Color color) {
			ScrollButton::normal_color = color;
			xscrollbar->setBackgroundColor(color);
			yscrollbar->setBackgroundColor(color);
			resizebtn->setBackgroundColor(color);
		}

		sf::Color getButtonNormalColor() const {
			return ScrollButton::normal_color;
		}

		void setButtonHoverColor(sf::Color color) {
			ScrollButton::hover_color = color;
		}

		sf::Color getButtonHoverColor() const {
			return ScrollButton::hover_color;
		}

		void setButtonThickness(float thickness) {
			thickness = std::max(thickness, 2.0f);
			ScrollButton::thickness = thickness;
			xscrollbar->setHeight(thickness, true);
			xscrollbar->updateFromPanel();
			yscrollbar->setWidth(thickness, true);
			yscrollbar->updateFromPanel();
			resizebtn->setSize({ thickness, thickness }, true);
		}

		float getButtonThickness() const {
			return ScrollButton::thickness;
		}

		void setButtonSpacing(float spacing) {
			spacing = std::max(spacing, 0.0f);
			ScrollButton::spacing = spacing;
			xscrollbar->setYPositionStyle(PositionStyle::InsideBottom, spacing);
			xscrollbar->updateFromPanel();
			yscrollbar->setXPositionStyle(PositionStyle::InsideRight, spacing);
			yscrollbar->updateFromPanel();
		}

		float getButtonSpacing() const {
			return ScrollButton::spacing;
		}

		void setScrollSpeed(float speed) {
			scrollspeed = std::max(1.0f, speed);
		}

		float getScrollSpeed() const {
			return scrollspeed;
		}

	private:

		void onResize() override {
			clipping_container->setSize(size(), true);
			innercontent->setSize(size());
			innercontent->setMinSize(size());
		}

		void updatePositions() {
			clipping_container->setSize(size(), true);
			if (innercontent->width() >= width()) {
				innercontent->setLeft(std::max(
					innercontent->left(),
					-innercontent->width() + width()
				));
			} else {
				innercontent->setLeft(0.0f);
			}
			if (innercontent->height() >= height()) {
				innercontent->setTop(std::max(
					innercontent->top(),
					-innercontent->height() + height()
				));
			} else {
				innercontent->setTop(0.0f);
			}
			xscrollbar->updateFromPanel();
			yscrollbar->updateFromPanel();
		}

		bool onScroll(float x, float y) override {
			if (keyDown(Key::LShift) || keyDown(Key::RShift)) {
				moveInner(-y * scrollspeed, -x * scrollspeed);
			} else {
				moveInner(-x * scrollspeed, -y * scrollspeed);
			}
			return true;
		}

		bool onKeyDown(Key key) override {
			switch (key) {
				case Key::Left:
					moveInner(-scrollspeed, 0.0f);
					return true;
				case Key::Right:
					moveInner(scrollspeed, 0.0f);
					return true;
				case Key::Up:
					moveInner(0.0f, -scrollspeed);
					return true;
				case Key::Down:
					moveInner(0.0f, scrollspeed);
					return true;
				case Key::PageUp:
					if (keyDown(Key::LShift) || keyDown(Key::RShift)) {
						moveInner(-width(), 0.0f);
					} else {
						moveInner(0.0f, -height());
					}
					return true;
				case Key::PageDown:
					if (keyDown(Key::LShift) || keyDown(Key::RShift)) {
						moveInner(width(), 0.0f);
					} else {
						moveInner(0.0f, height());
					}
					return true;
				case Key::Home:
					gotoHome(keyDown(Key::LShift) || keyDown(Key::RShift));
					return true;
				case Key::End:
					gotoEnd(keyDown(Key::LShift) || keyDown(Key::RShift));
					return true;
				default:
					return false;
			}
		}

		void moveInner(float x, float y) {
			if (innercontent->width() > width()) {
				innercontent->setLeft(
					std::max(
						std::min(innercontent->left() - x, 0.0f),
						width() - innercontent->width()
					)
				);
			} else {
				innercontent->setLeft(0.0f);
			}

			if (innercontent->height() > height()) {
				innercontent->setTop(
					std::max(
						std::min(innercontent->top() - y, 0.0f),
						height() - innercontent->height()
					)
				);
			} else {
				innercontent->setTop(0.0f);
			}

			xscrollbar->updateFromPanel();
			yscrollbar->updateFromPanel();
		}

		void gotoHome(bool horizontal) {
			if (horizontal) {
				if (innercontent->width() > width()) {
					innercontent->setLeft(0.0f);
				}
			} else {
				if (innercontent->height() > height()) {
					innercontent->setTop(0.0f);
				}
			}
			xscrollbar->updateFromPanel();
			yscrollbar->updateFromPanel();
		}

		void gotoEnd(bool horizontal) {
			if (horizontal) {
				if (innercontent->width() > width()) {
					innercontent->setLeft(width() - innercontent->width());
				}
			} else {
				if (innercontent->height() > height()) {
					innercontent->setTop(height() - innercontent->height());
				}
			}
			xscrollbar->updateFromPanel();
			yscrollbar->updateFromPanel();
		}

		struct ScrollButton : ui::FreeElement {
			ScrollButton() {
				setBackgroundColor(normal_color);
			}

			void onMouseOver() override {
				fadeColor(backgroundColor(), hover_color);
			}

			void onMouseOut() override {
				fadeColor(backgroundColor(), normal_color);
			}

			bool onLeftClick(int) override {
				startDrag();
				return true;
			}

			void onLeftRelease() override {
				stopDrag();
			}

			void fadeColor(sf::Color from, sf::Color to) {
				auto self = shared_from_this();
				startTransition(0.25f, [=](float t) {
					self->setBackgroundColor(sf::Color(
						(uint8_t)(from.r * (1.0f - t) + to.r * t),
						(uint8_t)(from.g * (1.0f - t) + to.g * t),
						(uint8_t)(from.b * (1.0f - t) + to.b * t),
						(uint8_t)(from.a * (1.0f - t) + to.a * t)
					));
				});
			}

			static float thickness;
			static float spacing;
			static sf::Color hover_color;
			static sf::Color normal_color;
		};

		struct VerticalScrollBar : ScrollButton {
			VerticalScrollBar(ScrollPanel& _panel) : panel(_panel) {
				setXPositionStyle(PositionStyle::InsideRight, spacing);
				setWidth(thickness, true);
				updateFromPanel();
			}

			void onDrag() override {
				movePanel();
			}

			void updateFromPanel() {
				if (panel.innercontent->height() <= panel.height()) {
					setSize({ thickness, thickness }, true);
					setTop(0.0f);
					setVisible(false);
					return;
				}
				setVisible(true);
				float height_ratio = panel.height() / panel.innercontent->height();
				float pos_ratio = -panel.innercontent->top() / (panel.innercontent->height() - panel.height());
				float max_height = panel.height() - thickness - 3.0f * spacing;
				float min_height = thickness;
				setHeight(max_height * height_ratio + min_height * (1.0f - height_ratio), true);
				setTop(spacing + pos_ratio * (panel.height() - height() - thickness - 3.0f * spacing));
			}

			void movePanel() {
				setTop(std::min(std::max(top(), spacing), panel.height() - height() - thickness - 2.0f * spacing));
				float pos_ratio = (top() - spacing) / (panel.height() - height() - thickness - 3.0f * spacing);
				panel.innercontent->setTop(round(-pos_ratio * (panel.innercontent->height() - panel.height())));
			}

			ScrollPanel& panel;
		};

		struct HorizontalScrollBar : ScrollButton {
			HorizontalScrollBar(ScrollPanel& _panel) : panel(_panel) {
				setYPositionStyle(PositionStyle::InsideBottom, spacing);
				setHeight(thickness, true);
				updateFromPanel();
			}

			void onDrag() override {
				movePanel();
			}

			void updateFromPanel() {
				if (panel.innercontent->width() <= panel.width()) {
					setSize({ thickness, thickness }, true);
					setLeft(0.0f);
					setVisible(false);
					return;
				}
				setVisible(true);
				float width_ratio = panel.width() / panel.innercontent->width();
				float pos_ratio = -panel.innercontent->left() / (panel.innercontent->width() - panel.width());
				float max_width = panel.width() - thickness - 3.0f * spacing;
				float min_width = thickness;
				setWidth(max_width * width_ratio + min_width * (1.0f - width_ratio), true);
				setLeft(spacing + pos_ratio * (panel.width() - width() - thickness - 3.0f * spacing));
			}

			void movePanel() {
				setLeft(std::min(std::max(left(), spacing), panel.width() - width() - thickness - 2.0f * spacing));
				float pos_ratio = (left() - spacing) / (panel.width() - width() - thickness - 3.0f * spacing);
				panel.innercontent->setLeft(round(-pos_ratio * (panel.innercontent->width() - panel.width())));
			}

			ScrollPanel& panel;
		};

		struct ResizeButton : ScrollButton {
			ResizeButton(ScrollPanel& _panel) : panel(_panel) {
				setXPositionStyle(PositionStyle::InsideRight, spacing);
				setYPositionStyle(PositionStyle::InsideBottom, spacing);
				setSize({ thickness, thickness }, true);
			}

			void onDrag() override {
				panel.setSize(pos() + size() + vec2(spacing, spacing), true);
			}

			ScrollPanel& panel;
		};

		struct InnerContent : ui::FreeElement {
			InnerContent(ScrollPanel& _panel) : panel(_panel) {

			}

			void onResize() override {
				panel.updatePositions();
			}

			ScrollPanel& panel;
		};

		Ref<FreeElement> clipping_container;
		Ref<InnerContent> innercontent;
		Ref<VerticalScrollBar> yscrollbar;
		Ref<HorizontalScrollBar> xscrollbar;
		Ref<ResizeButton> resizebtn;

		static float scrollspeed;
	};

	float ScrollPanel::scrollspeed = 25.0f;
	float ScrollPanel::ScrollButton::thickness = 10.0f;
	float ScrollPanel::ScrollButton::spacing = 5.0f;
	sf::Color ScrollPanel::ScrollButton::hover_color { 0x808080FF };
	sf::Color ScrollPanel::ScrollButton::normal_color { 0x80808080 };

} // namespace ui