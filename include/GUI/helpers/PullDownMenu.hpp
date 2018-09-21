#pragma once

#ifndef TIMSGUI_PULLDOWNMENU_H
#define TIMSGUI_PULLDOWNMENU_H

#include "GUI/helpers/CallbackButton.hpp"

namespace ui {

	template<typename Type>
	struct PullDownMenu : CallbackButton {
		PullDownMenu(std::vector<std::pair<Type, std::string>> options, const sf::Font& font, std::function<void(const Type&)> _onSelect)
			: CallbackButton(
				options.empty() ? "" : options.front().second,
				font,
				[this] { this->showList(); }
			),
			onSelect(_onSelect) {

			setMinSize({ 0.0f, 20.0f });
			setBackgroundColor(sf::Color(0xBBBBBBFF));
			setBorderColor(sf::Color(0xFF));
			setBorderThickness(1.0f);

			list = add<FreeElement>();
			for (const auto& option : options) {
				Type value = option.first;
				std::string desc = option.second;
				auto item = list->add<ListItem>(desc, font, [this, desc, value] {
					this->setCaption(desc);
					if (this->onSelect) {
						this->onSelect(value);
					}
					this->hideList();
					this->grabFocus();
				});
				list_items.push_back(item);
			}
			hideList();
		}
		PullDownMenu(std::vector<Type> options, const sf::Font& font, std::function<void(const Type&)> _onSelect)
			: PullDownMenu(generateDescriptions(options), font, _onSelect) {
		}

	private:

		void showList() {
			grabFocus();
			bringToFront();
			adopt(list);
			list->grabFocus();
			for (const auto& item : list_items) {
				item->setNormalColor(getNormalColor());
				item->setHoverColor(getHoverColor());
				item->setActiveColor(getActiveColor());
			}
		}

		void hideList() {
			release(list);
		}

		bool onLeftClick(int) override {
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

		struct ListItem : ui::CallbackButton {
			ListItem(std::string text, const sf::Font& font, std::function<void()> callback) :
				CallbackButton(text, font, callback) {

				setLayoutStyle(LayoutStyle::Block);
				setMinSize({ 0.0f, 20.0f });
			}

			bool onKeyDown(ui::Key key) override {
				if (CallbackButton::onKeyDown(key)) {
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
		};

		Ref<FreeElement> list;
		std::vector<Ref<ListItem>> list_items;
		std::function<void(const Type&)> onSelect;
	};

} // namespace ui

#endif // TIMSGUI_PULLDOWNMENU_H