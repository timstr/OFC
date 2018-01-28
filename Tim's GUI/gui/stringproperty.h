#pragma once

#include "formsmodel.h"
#include "formscontrol.h"

namespace ui {
	namespace forms {

		typedef PropertyTemplate<std::string> StringProperty;

		template<>
		struct PropertyTemplate<std::string> : TypeProperty<std::string> {
			StringProperty(std::string str) : TypeProperty(str) {

			}

			Control* makeControl(const sf::Font& font) override {
				return new StringControl(*this, font);
			}

			private:

			struct StringControl : Control {
				StringControl(StringProperty& _strprop, const sf::Font& font) : strprop(_strprop) {
					text = new TextEntry(strprop.value, font);
					addChildWindow(text);
					size = text->size;
				}

				void submit() override {
					strprop.value = text->getText();
				}

				void render(sf::RenderWindow& rw) override {
					size = text->size;
					renderChildWindows(rw);
				}

				private:
				TextEntry* text;
				StringProperty& strprop;
			};
		};

	}
}