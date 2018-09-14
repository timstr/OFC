#pragma once

#ifndef TIMSGUI_SLIDER_H
#define TIMSGUI_SLIDER_H

#include "GUI/Element.hpp"

namespace ui {

	struct Slider : InlineElement {
		Slider(float defaultval, float min, float max, sf::Font& font, std::function<void(float)> _onChange)
			: minimum(std::min(min, max)),
			maximum(std::max(min, max)),
			value(std::min(std::max(defaultval, minimum), maximum)),
			onChange(_onChange) {

			setSize({ 100.0f, 30.0f }, true);
			setBackgroundColor(sf::Color(0xFFFFFFFF));
			label = add<Text>(std::to_wstring(value), font);
			handle = add<Handle>(*this);
		}

		bool onLeftClick(int) override {
			handle->setPos(localMousePos() - handle->size() * 0.5f);
			handle->startDrag();
			return true;
		}

		void onLeftRelease() override {
			handle->stopDrag();
		}

		void setMinimum(float min) {
			minimum = min;
		}
		float getMinimum() const {
			return minimum;
		}

		void setMaximum(float max) {
			maximum = max;
		}
		float getMaximum() {
			return maximum;
		}

		void setValue(float val) {
			value = std::min(std::max(minimum, val), maximum);
			label->setText(std::to_string(val));
			moveHandleTo(value);
		}
		float getValue() const {
			return value;
		}

		const Ref<Element>& getHandle() const {
			return handle;
		}

	private:

		void onResize() override {
			handle->setSize({ height(), height() });
			moveHandleTo(value);
		}

		void moveHandleTo(float val) {
			float x = (val - minimum) / (maximum - minimum) * (width() - handle->width());
			handle->setPos({ x, 0 });
		}

		bool onKeyDown(ui::Key key) override {
			vec2 delta = (keyDown(ui::Key::LShift) || keyDown(ui::Key::RShift)) ? vec2(0.1f, 0.0f) : vec2(1.0f, 0.0f);
			if (key == ui::Key::Left) {
				handle->setPos(handle->pos() - delta);
				handle->updateFromPos();
				return true;
			}
			if (key == ui::Key::Right) {
				handle->setPos(handle->pos() + delta);
				handle->updateFromPos();
				return true;
			}
			return false;
		}

		struct Handle : FreeElement {
			Handle(Slider& _slider) : slider(_slider), reference_position(0.0f), fine_dragging(false) {
				float s = slider.height();
				setSize({ s, s }, true);
				setBackgroundColor(sf::Color(0x80808080));
				setBorderThickness(0.0f);
			}

			bool onLeftClick(int) override {
				fine_dragging = false;
				startDrag();
				return true;
			}

			void onLeftRelease() override {
				stopDrag();
			}

			bool onRightClick(int) override {
				fine_dragging = true;
				reference_position = left();
				startDrag();
				return true;
			}

			void onRightRelease() override {
				stopDrag();
			}

			void onDrag() override {
				if (fine_dragging) {
					float diff = left() - reference_position;
					setLeft(reference_position + fine_speed * diff);
				}
				updateFromPos();
			}

			bool onKeyDown(ui::Key key) override {
				vec2 delta = (keyDown(ui::Key::LShift) || keyDown(ui::Key::RShift)) ? vec2(0.1f, 0.0f) : vec2(1.0f, 0.0f);
				if (key == ui::Key::Left) {
					setPos(pos() - delta);
					updateFromPos();
					return true;
				}
				if (key == ui::Key::Right) {
					setPos(pos() + delta);
					updateFromPos();
					return true;
				}
				return false;
			}

			void updateFromPos() {
				vec2 p = pos();
				float left = 0.0f;
				float right = slider.width() - width();
				p.x = std::min(std::max(p.x, left), right);
				p.y = 0.0f;
				setPos(p);
				float x = (p.x - left) / (right - left);
				slider.value = x * (slider.maximum - slider.minimum) + slider.minimum;
				slider.label->setText(std::to_string(slider.value));
				slider.onChange(slider.value);
			}

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

	const float Slider::Handle::fine_speed = 0.1f;

} // namespace ui

#endif // TIMSGUI_SLIDER_H