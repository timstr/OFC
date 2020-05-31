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

    auto str= ui::Property<ui::String>("Dagnabbit darn drat!");

	auto condition = ui::Property<bool>{true};

	ui::AnyComponent comp = ui::UseFont(&getFont()).with(
        ui::If(condition).then(
            ui::Button().caption("Click Me!").onClick([&](){ condition.set(false); })
        ).otherwise(
            "Dagnabbit darn drat.."
        )
    );

    auto root = ui::ComponentRoot::create<ui::FreeContainer>(std::move(comp));

    ui::Window& win = ui::Window::create(std::move(root), 600, 400, "Tim's GUI Test");

	ui::run();

	return 0;
}