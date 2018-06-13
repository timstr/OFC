#pragma once

#include "gui/element.h"

namespace ui {

	template<typename Type>
	struct PullDownMenu : InlineElement {
		PullDownMenu(std::vector<std::pair<Type, std::string>> options, sf::Font& font, std::function<void(const Type&)> _onSelect)
			: collapsed(true), onSelect(_onSelect) {
			setMinSize({0.0f, 20.0f});

			caption = add<Text>(options.empty() ? "" : options.front().second, font);

			list = add<FreeElement>();
			list->setPadding(0.0f);
			list->setVisible(false);
			for (const auto& option : options){
				Type value = option.first;
				std::string desc = option.second;
				list->add<ListItem>(desc, font, [this,desc,value]{
					this->caption->setText(desc);
					if (this->onSelect){
						this->onSelect(value);
					}
					this->list->setVisible(false);
					this->grabFocus();
				});
			}
		}
		PullDownMenu(std::vector<Type> options, sf::Font& font, std::function<void(const Type&)> _onSelect)
			: PullDownMenu(generateDescriptions(options), font, _onSelect) {
		}

		bool onLeftClick(int clicks) override {
			list->setVisible(true);
			return true;
		}

		bool onLeftRelease() override {
			return true;
		}

		void onLoseFocus() override {
			list->setVisible(false);
		}

		void render(sf::RenderWindow& rw) override {
			sf::RectangleShape rect {getSize()};
			rect.setFillColor(sf::Color(0xBBBBBBFF));
			rect.setOutlineColor(sf::Color(0xFF));
			rect.setOutlineThickness(1.0f);
			rw.draw(rect);
		}

	private:

		std::vector<std::pair<Type, std::string>> generateDescriptions(std::vector<Type> options){
			std::vector<std::pair<Type, std::string>> res;
			res.reserve(options.size());
			for (const auto& option : options){
				res.emplace_back(std::make_pair(option, static_cast<std::string>(option)));
			}
			return res;
		}

		struct ListItem : BlockElement {
			ListItem(std::string _text, sf::Font& _font, std::function<void()> _callback) : callback(_callback), bgcolor(0xBBBBBBFF) {
				add<Text>(_text, _font);
				setMinSize({0.0f, 20.0f});
				setMargin(0.0f);
			}

			void onMouseOver() override {
				fadeColor(bgcolor, sf::Color(0xDDDDDDFF), 0.15f);
			}
			void onMouseOut() override {
				fadeColor(bgcolor, sf::Color(0xBBBBBBFF), 0.15f);
			}

			bool onLeftClick(int clicks) override {
				if (callback){
					callback();
				}
				return true;
			}

			void render(sf::RenderWindow& rw) override {
				sf::RectangleShape rect {getSize()};
				rect.setFillColor(bgcolor);
				rect.setOutlineColor(sf::Color(0xFF));
				rect.setOutlineThickness(1.0f);
				rw.draw(rect);
			}

			void fadeColor(sf::Color from, sf::Color to, float seconds){
				auto self = getThisAs<ListItem>();
				startTransition(seconds, [=](float t){
					auto color = sf::Color(
						(uint8_t)(from.r * (1.0f - t) + to.r * t),
						(uint8_t)(from.g * (1.0f - t) + to.g * t),
						(uint8_t)(from.b * (1.0f - t) + to.b * t),
						255
					);
					self->bgcolor = color;
				});
			}

			sf::Color bgcolor;
			const std::function<void()> callback;
		};

		bool collapsed;
		std::shared_ptr<Text> caption;
		std::shared_ptr<FreeElement> list;
		std::function<void(const Type&)> onSelect;
	};

} // namespace ui