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
					caption = new ui::Text(prop.items.size() > 0 ? prop.items[0].second : "-", font);
					caption->setBackGroundColor(sf::Color(0xFFFFFFFF));
					size = caption->size;
					addChildWindow(caption);
					list = new List(*this, font);
					addChildWindow(list, below(caption));
					list->visible = false;
				}

				void submit() override {

				}

				void onLeftClick(int clicks) override {
					list->visible = true;
				}

				struct Item : Window {
					Item(PullDownControl& _control, const Type& _val, const std::string& _str, const sf::Font& font)
						: control(_control), val(_val), str(_str) {
						ui::Text* text = new ui::Text(str, font);
						text->setBackGroundColor(sf::Color(0xFFFFFFFF));
						addChildWindow(text);
						size = text->size;
					}

					void onLeftClick(int clicks) override {
						control.prop.value = val;
						control.caption->setText(str);
						control.list->visible = false;
					}

					const Type& val;
					const std::string& str;
					PullDownControl& control;
				};

				struct List : Window {
					List(PullDownControl& control, const sf::Font& font){
						float ypos = 0.0f;
						for (const auto& it : control.prop.items){
							Item* item = new Item(control, it.first, it.second, font);
							addChildWindow(item);
							item->pos.y = ypos;
							ypos += item->size.y;
						}
					}
				};

				ui::Text* caption;
				List* list;
				PullDownProperty& prop;
			};
		};

	}
}