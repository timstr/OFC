#pragma once

#include "gui/element.h"

namespace ui {

	struct LineBreak final : BlockElement {
		LineBreak(float height = 0.0f){
			setMargin(height * 0.5f);
			disable();
		}

		void render(sf::RenderWindow& rw) override {

		}
	};

} // namespace ui