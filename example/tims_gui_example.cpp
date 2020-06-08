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

    using namespace ui;

    // auto str = Property<String>("Dagnabbit darn drat!");

	auto vec = Property{std::vector<int>{0, 1, 2, 3, 4}};

	AnyComponent comp = UseFont(&getFont()).with(
		List(
			Button("Clear All").onClick([&vec] {
				vec.getOnceMutable().clear();
			}),
			ForEach(vec).Do([&](int i, const std::size_t& idx) -> AnyComponent {
				return Button(std::to_string(i)).onClick([&vec,&idx]{
					auto& v = vec.getOnceMutable();
					v.erase(v.begin() + idx);
				});
			}),
			Button("Add One More").onClick([&vec] {
				auto& v = vec.getOnceMutable();
				v.push_back(v.empty() ? 0 : (v.back() + 1));
			}),
			Button("What??").onClick([&vec] {
				const auto& v = vec.getOnce();
				for (const auto& x : v) {
					std::cout << x << ' ';
				}
				std::cout << '\n';
			})
		)
	);

	auto root = ComponentRoot::create<FlowContainer>(std::move(comp));

	/*
	auto condition = Property<bool>{true};

	AnyComponent comp = UseFont(&getFont()).with(
        If(condition).then(
            Button().caption("Click Me!").onClick([&](){ condition.set(false); })
        ).otherwise(
            "Dagnabbit darn drat.."
        )
    );
	auto root = ComponentRoot::create<FreeContainer>(std::move(comp));
	*/

    

    Window& win = Window::create(std::move(root), 600, 400, "Tim's GUI Test");

	run();

	return 0;
}