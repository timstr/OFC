#pragma once

#include "formscontrol.h"
#include "formsmodel.h"
#include "helpers.h"

namespace ui {
	namespace forms {

		typedef PropertyTemplate<int> IntegerProperty;

		template<>
		struct PropertyTemplate<int> : TypeProperty<int> {
			PropertyTemplate(int val) : TypeProperty(val) {

			}

			Control* makeControl(const sf::Font& font) override {
				return new IntegerControl(*this, font);
			}

			private:

			struct IntegerControl : Control {
				IntegerControl(IntegerProperty& intprop, const sf::Font& font) : prop(intprop) {
					auto validate = [](const std::string& str) -> bool {
						return stringIsInt(str);
					};


					text = new helpers::TextEntryHelper(toString(prop.value), font, {}, validate);
					addChildWindow(text);
					size = text->size;
				}

				void submit() override {
					prop.value = stringToInt(text->getText(), 0);
				}

				void render(sf::RenderWindow& rw) override {
					size = text->size;
					renderChildWindows(rw);
				}

				private:
				helpers::TextEntryHelper* text;
				IntegerProperty& prop;
			};
		};
	}
}