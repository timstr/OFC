#pragma once

#include "gui/element.h"

namespace ui {

	// TODO: think of a better name
	struct LineBreak final : BlockElement {
		LineBreak(float height = 0.0f){
			setMargin(height * 0.5f);
			disable();
			setSize({0.0f, 0.0f}, true);
		}

		void render(sf::RenderWindow& rw) override {

		}
	};

} // namespace ui