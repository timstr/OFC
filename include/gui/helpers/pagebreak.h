#pragma once

#include "gui/element.h"

namespace ui {

	struct PageBreak final : BlockElement {
		PageBreak(float height = 0.0f){
			setPadding(0.0f);
			setMargin(height * 0.5f);
			setBorderColor(sf::Color(0));
			setBackgroundColor(sf::Color(0));
			disable();
			setSize({0.0f, 0.0f});
		}
	};

} // namespace ui