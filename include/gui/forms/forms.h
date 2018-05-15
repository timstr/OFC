#pragma once

#include <type_traits>
#include "formscontrol.h"
#include "formsmodel.h"
#include "gui/helpers.h"

#include "stringproperty.h"
#include "floatproperty.h"
#include "doubleproperty.h"
#include "integerproperty.h"
#include "pulldownproperty.h"

// TODO: add toggle button, sliders

namespace ui {
	namespace forms {
		
		// TODO:
		// tab navigation, space/enter toggling, automatic alignment and sizing
		// TODO: add cancel button

		struct Form : Window {
			Form(Model _model, const sf::Font& font);

			void onSubmit(const std::function<void(Model)>& _on_submit);

			void submit();

			private:

			Model model;
			std::function<void(Model)> on_submit;
			std::vector<Control*> controls;
			friend struct Control;
		};

	}
}