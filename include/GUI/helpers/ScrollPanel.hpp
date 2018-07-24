#pragma once

#include "GUI/Element.hpp"
#include "GUI/GUI.hpp"

namespace ui {

	struct ScrollPanel : ui::InlineElement {
		ScrollPanel(bool horizontal_scrolling = true, bool vertical_scrolling = true, bool resize_button = false) {
			setClipping(true);
			innercontent = add<InnerContent>(*this);
			if (horizontal_scrolling) {
				xscrollbar = add<HorizontalScrollBar>(*this);
			}
			if (vertical_scrolling) {
				yscrollbar = add<VerticalScrollBar>(*this);
			}
		}

		Ref<Element> inner() const {
			return innercontent;
		}

	private:

		void onResize() override {
			innercontent->setMinSize(size());
			if (innercontent->width() > width()) {
				innercontent->setLeft(std::max(
					innercontent->left(),
					-innercontent->width() + width()
				));
			} else {
				innercontent->setLeft(0.0f);
			}
			if (innercontent->height() > height()) {
				innercontent->setTop(std::max(
					innercontent->top(),
					-innercontent->height() + height()
				));
			} else {
				innercontent->setTop(0.0f);
			}
			if (xscrollbar) {
				xscrollbar->updateFromPanel();
			}
			if (yscrollbar) {
				yscrollbar->updateFromPanel();
			}
		}

		bool onScroll(float x, float y) override {
			if (innercontent->width() > width()) {
				innercontent->setLeft(
					std::max(
						std::min(innercontent->left() + x * scrollspeed, 0.0f),
						width() - innercontent->width()
					)
				);
			} else {
				innercontent->setLeft(0.0f);
			}

			if (innercontent->height() > height()) {
				innercontent->setTop(
					std::max(
						std::min(innercontent->top() + y * scrollspeed, 0.0f),
						height() - innercontent->height()
					)
				);
			} else {
				innercontent->setTop(0.0f);
			}

			xscrollbar->updateFromPanel();
			yscrollbar->updateFromPanel();

			return true;
		}

		struct ScrollBar : ui::FreeElement {
			ScrollBar() {
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

			static const float thickness;
			static const float spacing;
			static const sf::Color hover_color;
			static const sf::Color normal_color;
		};

		struct VerticalScrollBar : ScrollBar {
			VerticalScrollBar(ScrollPanel& _panel) : panel(_panel) {
				setXPositionStyle(PositionStyle::InsideLeft, spacing);
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
				setHeight(panel.height() * height_ratio, true);
				setTop(pos_ratio * (panel.height() - height()));
			}

			void movePanel() {
				setLeft(spacing);
				setTop(std::min(std::max(top(), 0.0f), panel.height() - height()));
				float pos_ratio = top() / (panel.height() - height());
				panel.innercontent->setTop(-pos_ratio * (panel.innercontent->height() - panel.height()));
			}

			ScrollPanel& panel;
		};

		struct HorizontalScrollBar : ScrollBar {
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
				setWidth(panel.width() * width_ratio, true);
				setLeft(pos_ratio * (panel.width() - width()));
			}

			void movePanel() {
				setLeft(std::min(std::max(left(), 0.0f), panel.width() - width()));
				setTop(panel.height() - height() - spacing);
				float pos_ratio = left() / (panel.width() - width());
				panel.innercontent->setLeft(-pos_ratio * (panel.innercontent->width() - panel.width()));
			}

			ScrollPanel& panel;
		};

		// TODO: size dragger (add if show_resizebtn)

		struct InnerContent : ui::FreeElement {
			InnerContent(ScrollPanel& _panel) : panel(_panel) {

			}

			void onResize() override {
				panel.onResize();
			}

			ScrollPanel& panel;
		};

		Ref<InnerContent> innercontent;
		Ref<VerticalScrollBar> yscrollbar;
		Ref<HorizontalScrollBar> xscrollbar;

		static const float scrollspeed;
	};

	const float ScrollPanel::scrollspeed = 10.0f;
	const float ScrollPanel::ScrollBar::thickness = 10.0f;
	const float ScrollPanel::ScrollBar::spacing = 5.0f;
	const sf::Color ScrollPanel::ScrollBar::hover_color { 0x808080FF };
	const sf::Color ScrollPanel::ScrollBar::normal_color { 0x80808080 };

} // namespace ui