#pragma once

#include "gui.h"
#include "helpers.h"

namespace ui {
	namespace forms {

		// Control is a user interface element for manipulating a model's property
		struct Control : ui::Window {
			virtual void submit() = 0;
		};

	}
}