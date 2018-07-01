#pragma once

#include "gui/element.h"

namespace ui {

	struct Slider : InlineElement {
		Slider(float defaultval, float min, float max, sf::Font& font, std::function<void(float)> _onChange)
			: minimum(std::min(min, max)),
			maximum(std::max(min, max)),
			value(std::min(std::max(defaultval, minimum), maximum)),
			onChange(_onChange) {

			setSize({100.0f, 35.0f}, true);
			label = add<Text>(std::to_wstring(value), font);
			handle = add<Handle>(*this);
		}

		bool onLeftClick(int clicks) override {
			handle->setPos(localMousePos() - handle->getSize() * 0.5f);
			handle->startDrag();
			return true;
		}

		void setMinimum(float min){
			minimum = min;
		}
		float getMinimum() const {
			return minimum;
		}

		void setMaximum(float max){
			maximum = max;
		}
		float getMaximum(float max){
			return maximum;
		}

		void setValue(float val){
			value = std::min(std::max(minimum, val), maximum);
			label->setText(std::to_string(val));
		}
		float getValue() const {
			return value;
		}

	private:
		struct Handle : FreeElement {
			Handle(Slider& _slider) : slider(_slider) {
				float size = slider.getSize().y;
				setSize({size, size}, true);
			}

			bool onLeftClick(int clicks) override {
				startDrag();
				return true;
			}

			void onLeftRelease() override {
				stopDrag();
			}

			void render(sf::RenderWindow& rw) override {
				sf::RectangleShape rect {getSize() - vec2(2.0f, 2.0f)};
				rect.setPosition({1.0f, 1.0f});
				rect.setFillColor(sf::Color(0x80808080));
				rect.setOutlineColor(sf::Color(0xFF));
				rect.setOutlineThickness(1.0f);
				rw.draw(rect);
			}

			void onDrag() override {
				vec2 pos = getPos();
				float left = 0.0f;
				float right = slider.getSize().x - getSize().x;
				pos.x = std::min(std::max(pos.x, left), right);
				pos.y = 0.0f;
				setPos(pos);
				float x = (pos.x - left) / (right - left);
				slider.value = x * (slider.maximum - slider.minimum) + slider.minimum;
				slider.label->setText(std::to_string(slider.value));
				slider.onChange(slider.value);
			}

			Slider& slider;
		};

		std::shared_ptr<Handle> handle;
		std::shared_ptr<Text> label;
		float minimum, maximum, value;
		std::function<void(float)> onChange;
	};

} // namespace ui