#pragma once

#include "gui/window.h"

namespace ui {


	struct Divider : Window {
		Divider(float _margin = 5.0f) : margin(_margin) {
			display_style = DisplayStyle::Block;
		}

		float getMargin() const {
			return margin;
		}

		void setMargin(float _margin){
			margin = std::max(_margin, 0.0f);
			onChange();
		}

		void onChange() override {
			float ypos = margin;
			float next_ypos = ypos;
			float xpos = margin;

			for (const auto& window : getChildWindows()){
				auto win = window.lock();
				if (!win || win->display_style == DisplayStyle::Free){
					continue;
				}
				if (win->display_style == DisplayStyle::Block){
					xpos = margin;
					win->setPos(vec2(xpos, next_ypos));
					win->setSize(vec2(this->getSize().x - 2.0f * margin, win->getSize().y));
					ypos = next_ypos + win->getSize().y + margin;
					next_ypos = ypos;
					
				} else if (win->display_style == DisplayStyle::Inline){
					win->setPos(vec2(xpos, ypos));
					xpos += win->getSize().x + margin;
					if (xpos > this->getSize().x && xpos > margin){
						xpos = 2.0f * margin + win->getSize().x;
						ypos = next_ypos;
						win->setPos(vec2(margin, ypos));
					}
					next_ypos = std::max(next_ypos, ypos + win->getSize().y + margin);
				}
			}

			setSize({getSize().x, next_ypos});
		}

	private:
		float margin;
	};

} // namespace ui
