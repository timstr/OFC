#pragma once

#include "formsmodel.h"
#include "formscontrol.h"

namespace ui {
	namespace forms {

		template<typename Type>
		struct PullDownProperty : TypeProperty<Type> {
			PullDownProperty(std::vector<std::pair<Type, std::string>> value_strings) : items(value_strings) {

			}
			PullDownProperty(std::vector<Type> values, std::function<std::string(Type)> _to_string){
				for (const auto& val : values){
					items.push_back(std::make_pair(val, _to_string(val)));
				}
			}

			Control* makeControl(const sf::Font& font) override {
				return new PullDownControl(*this, font);
			}

			private:

			std::vector<std::pair<Type, std::string>> items;

			struct PullDownControl : Control {
				PullDownControl(PullDownProperty& _prop, const sf::Font& font) : prop(_prop) {
					const float width = 100.0f;
					caption = new ui::Text(prop.items.size() > 0 ? prop.items[0].second : "-", font);
					caption->setBackGroundColor(sf::Color(0xFFFFFFFF));
					size.x = width;
					caption->size.x = width;
					size.y = caption->size.y;
					addChildWindow(caption);
					list = new List(*this, font, width);
					addChildWindow(list, below(caption));
					list->visible = false;
				}

				void submit() override {

				}

				void onLeftClick(int clicks) override {
					list->visible = !list->visible;
					if (list->visible){
						list->grabFocus();
					}
				}

				struct Item : Window {
					Item(PullDownControl& _control, const Type& _val, const std::string& _str, const sf::Font& font, float width)
						: control(_control), val(_val), str(_str) {
						ui::Text* text = new ui::Text(str, font);
						text->setBackGroundColor(sf::Color(0xFFFFFFFF));
						addChildWindow(text);
						size.x = width;
						text->size.x = width;
						text->size.y += 2.0f;
						size.y = text->size.y;
					}

					void onLeftClick(int clicks) override {
						control.prop.value = val;
						control.caption->setText(str);
						control.caption->size.x = size.x;
						control.list->visible = false;
					}

					const Type& val;
					const std::string& str;
					PullDownControl& control;
				};

				struct List : Window {
					List(PullDownControl& control, const sf::Font& font, float width){
						float ypos = 0.0f;
						for (const auto& it : control.prop.items){
							Item* item = new Item(control, it.first, it.second, font, width);
							addChildWindow(item);
							item->pos.y = ypos;
							ypos += item->size.y;
						}
					}

					void onLoseFocus() override {
						visible = false;
					}
				};

				ui::Text* caption;
				List* list;
				PullDownProperty& prop;
			};
		};

	}
}