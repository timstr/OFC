#pragma once

#include "formscontrol.h"
#include "formsmodel.h"
#include "helpers.h"

namespace ui {
	namespace forms {

		struct FloatProperty : TypeProperty<float> {
			FloatProperty(float val) : TypeProperty(val) {

			}

			Control* makeControl(const sf::Font& font) override;
		};

		struct FloatControl : Control {
			FloatControl(FloatProperty& dprop, const sf::Font& font) : prop(dprop) {
				auto validate = [](const std::string& str) -> bool {
					return !std::isnan(stringToFloat(str));
				};


				text = new helpers::TextEntryHelper(toString(prop.value), font, {}, validate);
				addChildWindow(text);
				size = text->size;
			}

			void submit() override {
				prop.value = stringToFloat(text->getText());
			}

			void render(sf::RenderWindow& rw) override {
				size = text->size;
				renderChildWindows(rw);
			}

			private:
			helpers::TextEntryHelper* text;
			FloatProperty& prop;
		};

		Control* FloatProperty::makeControl(const sf::Font& font){
			return new FloatControl(*this, font);
		}
	}
}