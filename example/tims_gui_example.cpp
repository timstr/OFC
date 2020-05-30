#include <GUI/GUI.hpp>
#include <GUI/Helpers.hpp>
#include <GUI/Component.hpp>

#include <iostream>
#include <random>
#include <sstream>

#include "fontspath.hpp"

std::random_device randdev;
std::mt19937 randeng { randdev() };

const sf::Font& getFont() {
	static sf::Font font;
	static bool loaded;
	if (!loaded) {
		font.loadFromFile(fonts_path + "/JosefinSans-Bold.ttf");
		loaded = true;
	}
	return font;
}

int main(){

    // auto caption = ui::Property<ui::String>("Click me!");

	auto condition = ui::Property<bool>{true};

	ui::AnyComponent comp = ui::If(condition)
		.then(ui::Button(getFont())
			.caption("Click Me!")
			.onClick([&](){ std::cout << "Clicked!\n"; condition.set(false); })
		).otherwise(
			ui::StaticText(getFont(), "Darn")
		);

    auto root = ui::ComponentRoot::create<ui::FreeContainer>(std::move(comp));

    ui::Window& win = ui::Window::create(std::move(root), 600, 400, "Tim's GUI Test");

	ui::run();

	return 0;
}