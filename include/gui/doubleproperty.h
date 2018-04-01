#pragma once

#include "formscontrol.h"
#include "formsmodel.h"
#include "helpers.h"

namespace ui {
	namespace forms {

		typedef PropertyTemplate<double> DoubleProperty;

		template<>
		struct PropertyTemplate<double> : TypeProperty<double> {
			PropertyTemplate(double val) : TypeProperty(val) {

			}

			Control* makeControl(const sf::Font& font) override {
				return new DoubleControl(*this, font);
			}

			private:

			struct DoubleControl : Control {
				DoubleControl(DoubleProperty& dprop, const sf::Font& font) : prop(dprop) {
					auto validate = [](const std::string& str) -> bool {
						return !std::isnan(stringToDouble(str));
					};


					text = new helpers::TextEntryHelper(toString(prop.value), font, {}, validate);
					addChildWindow(text);
					size = text->size;
				}

				void submit() override {
					prop.value = stringToDouble(text->getText());
				}

				void render(sf::RenderWindow& rw) override {
					size = text->size;
					renderChildWindows(rw);
				}

				private:
				helpers::TextEntryHelper* text;
				DoubleProperty& prop;
			};
		};

	}
}