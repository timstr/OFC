#include "gui/forms.h"

namespace ui {
	namespace forms {

		Form::Form(Model _model, const sf::Font& font) : model(_model) {
			// TODO
		}

		void Form::onSubmit(const std::function<void(Model)>& _on_submit){
			on_submit = _on_submit;
		}

		void Form::submit(){
			for (Control* control: controls){
				control->submit();
			}
			if (on_submit){
				on_submit(model);
			}
		}

	} // namespace forms
} // namespace ui