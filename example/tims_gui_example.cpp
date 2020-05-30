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
    
    ui::Window& win = ui::Window::create(1000, 800, "Tim's GUI Test");

	auto condition = ui::Property<bool>{true};
	auto caption = ui::Property<ui::String>("Click me!");

	ui::AnyComponent comp = ui::If(condition)
		.then(ui::Button(getFont())
			.caption(caption)
			.onClick([](){ std::cout << "Clicked!\n"; })
		).otherwise(
			ui::StaticText(getFont(), "Darn")
		);

	ui::run();

	return 0;
}