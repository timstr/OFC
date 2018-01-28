#pragma once

#include <type_traits>
#include "formscontrol.h"
#include "formsmodel.h"
#include "helpers.h"

#include "stringproperty.h"
#include "floatproperty.h"
#include "doubleproperty.h"
#include "integerproperty.h"

// TODO: add toggle button, sliders, pull-down list

namespace ui {
	namespace forms {
		
		// TODO:
		// tab navigation, space/enter toggling, automatic alignment and sizing

		struct Form : Window {
			Form(Model _model, const sf::Font& font) : model(_model) {
				size = {0, 5};
				float y = 5;
				for (auto it = model.properties.begin(); it != model.properties.end(); it++){
					Text* caption = new Text(it->first, font);
					Control* control = it->second->makeControl(font);
					controls.push_back(control);
					addChildWindow(caption, insideLeft(this, 5), y);
					addChildWindow(control, rightOf(caption, 5), y);
					size.x = std::max(size.x, caption->size.x + control->size.x + 15);
					size.y += control->size.y + 5;
					y += control->size.y + 5;
				}
				auto submitbtn = new ui::helpers::CallbackButton("Submit", font, [this](){
					this->submit();
					this->close();
				});
				size.x = std::max(size.x, submitbtn->size.x + 10);
				size.y += submitbtn->size.y + 5;
				addChildWindow(submitbtn, middleOfX(this), y);
			}

			void onSubmit(const std::function<void(Model)>& _on_submit){
				on_submit = _on_submit;
			}

			void submit(){
				for (Control* control: controls){
					control->submit();
				}
				if (on_submit){
					on_submit(model);
				}
			}

			private:

			Model model;
			std::function<void(Model)> on_submit;
			std::vector<Control*> controls;
			friend struct Control;
		};

	}
}