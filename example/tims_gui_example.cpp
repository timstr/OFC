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

    // auto str = ui::Property<ui::String>("Dagnabbit darn drat!");

	auto vec = ui::Property{std::vector<int>{0, 1, 2, 3, 4}};

	ui::AnyComponent comp = ui::UseFont(&getFont()).with(
		ui::List(
			ui::Button().caption("Clear All").onClick([&vec] {
				vec.getOnceMutable().clear();
			}),
			ui::ForEach(vec).Do([&](int i, const std::size_t& idx) -> ui::AnyComponent {
				return ui::Button().caption(std::to_string(i)).onClick([&vec,&idx]{
					auto& v = vec.getOnceMutable();
					v.erase(v.begin() + idx);
				});
			}),
			ui::Button().caption("Add One More").onClick([&vec] {
				auto& v = vec.getOnceMutable();
				v.push_back(v.empty() ? 0 : (v.back() + 1));
			}),
			ui::Button().caption("What??").onClick([&vec] {
				const auto& v = vec.getOnce();
				for (const auto& x : v) {
					std::cout << x << ' ';
				}
				std::cout << '\n';
			})
		)
	);

	auto root = ui::ComponentRoot::create<ui::FlowContainer>(std::move(comp));

	/*
	auto condition = ui::Property<bool>{true};

	ui::AnyComponent comp = ui::UseFont(&getFont()).with(
        ui::If(condition).then(
            ui::Button().caption("Click Me!").onClick([&](){ condition.set(false); })
        ).otherwise(
            "Dagnabbit darn drat.."
        )
    );
	auto root = ui::ComponentRoot::create<ui::FreeContainer>(std::move(comp));
	*/

    

    ui::Window& win = ui::Window::create(std::move(root), 600, 400, "Tim's GUI Test");

	ui::run();

	return 0;
}