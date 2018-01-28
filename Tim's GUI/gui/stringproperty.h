#pragma once

#include "formsmodel.h"
#include "formscontrol.h"

namespace ui {
	namespace forms {

		struct StringProperty : TypeProperty<std::string> {
			StringProperty(std::string str) : TypeProperty(str) {

			}

			Control* makeControl(const sf::Font& font) override;
		};

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

		Control* StringProperty::makeControl(const sf::Font& font){
			return new StringControl(*this, font);
		}

	}
}