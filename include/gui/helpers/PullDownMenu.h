#pragma once

#include "gui/element.h"

namespace ui {

	template<typename Type>
	struct PullDownMenu : InlineElement {
		PullDownMenu(std::vector<std::pair<Type, std::string>> options, sf::Font& font, std::function<void(const Type&)> _onSelect)
			: onSelect(_onSelect) {

			setMinSize({ 0.0f, 20.0f });
			setBackgroundColor(sf::Color(0xBBBBBBFF));
			setBorderColor(sf::Color(0xFF));
			setBorderThickness(1.0f);

			caption = add<Text>(options.empty() ? "" : options.front().second, font);

			list = add<FreeElement>();
			for (const auto& option : options) {
				Type value = option.first;
				std::string desc = option.second;
				list->add<ListItem>(desc, font, [this, desc, value] {
					this->caption->setText(desc);
					if (this->onSelect) {
						this->onSelect(value);
					}
					this->hideList();
					this->grabFocus();
				});
			}
			hideList();
		}
		PullDownMenu(std::vector<Type> options, sf::Font& font, std::function<void(const Type&)> _onSelect)
			: PullDownMenu(generateDescriptions(options), font, _onSelect) {
		}

	private:

		void showList() {
			grabFocus();
			bringToFront();
			adopt(list);
			list->grabFocus();
		}

		void hideList() {
			release(list);
		}

		bool onLeftClick(int clicks) override {
			showList();
			return true;
		}

		bool onKeyDown(ui::Key key) override {
			if (key == ui::Key::Return ||
				key == ui::Key::Left ||
				key == ui::Key::Right ||
				key == ui::Key::Up ||
				key == ui::Key::Down) {
				showList();
				list->navigateIn();
				return true;
			}
			if (key == ui::Key::Escape) {
				hideList();
			}
			return false;
		}

		void onLoseFocus() override {
			hideList();
		}

		std::vector<std::pair<Type, std::string>> generateDescriptions(std::vector<Type> options) {
			std::vector<std::pair<Type, std::string>> res;
			res.reserve(options.size());
			for (const auto& option : options) {
				res.emplace_back(std::make_pair(option, static_cast<std::string>(option)));
			}
			return res;
		}

		struct ListItem : BlockElement {
			ListItem(std::string _text, sf::Font& _font, std::function<void()> _callback) : callback(_callback) {
				add<Text>(_text, _font);
				setMinSize({ 0.0f, 20.0f });
				setBorderColor(sf::Color(0xFF));
				setBorderThickness(1.0f);
				setBackgroundColor(sf::Color(0xBBBBBBFF));
			}

			void onMouseOver() override {
				fadeColor(getBackgroundColor(), sf::Color(0xDDDDDDFF), 0.15f);
			}
			void onMouseOut() override {
				fadeColor(getBackgroundColor(), sf::Color(0xBBBBBBFF), 0.15f);
			}

			bool onLeftClick(int clicks) override {
				if (callback) {
					callback();
				}
				return true;
			}

			bool onKeyDown(ui::Key key) override {
				if (key == ui::Key::Return) {
					if (callback) {
						callback();
					}
					return true;
				}
				if (key == ui::Key::Left || key == ui::Key::Up) {
					navigateToPreviousElement();
					return true;
				}
				if (key == ui::Key::Right || key == ui::Key::Down) {
					navigateToNextElement();
					return true;
				}
				return false;
			}

			void fadeColor(sf::Color from, sf::Color to, float seconds) {
				auto self = getThisAs<ListItem>();
				startTransition(seconds, [=](float t) {
					auto color = sf::Color(
						(uint8_t)(from.r * (1.0f - t) + to.r * t),
						(uint8_t)(from.g * (1.0f - t) + to.g * t),
						(uint8_t)(from.b * (1.0f - t) + to.b * t),
						255
					);
					self->setBackgroundColor(color);
				});
			}

			const std::function<void()> callback;
		};

		std::shared_ptr<Text> caption;
		std::shared_ptr<FreeElement> list;
		std::function<void(const Type&)> onSelect;
	};

} // namespace ui