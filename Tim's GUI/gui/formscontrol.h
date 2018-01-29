#pragma once

#include "gui.h"
#include "helpers.h"

namespace ui {
	namespace forms {

		// Control is a user interface element for manipulating a model's property
		struct Control : ui::Window {
			Control(){
				bring_to_front = true;
			}

			virtual void submit() = 0;
		};

	}
}