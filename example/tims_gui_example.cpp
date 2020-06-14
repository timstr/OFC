#include <GUI/GUI.hpp>

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
/*
struct PulldownMenuState {
    PulldownMenuState()
        : currentIndex(0)
        , expanded(false) {

    }

    ui::Property<std::size_t> currentIndex;
    ui::Property<bool> expanded;
};

class PulldownMenu : public ui::StatefulComponent<PulldownMenuState> {
public:
    PulldownMenu(ui::PropertyOrValue<std::vector<ui::String>> items)
        :  m_items(std::move(items)) {

    }

    PulldownMenu& onChange(std::function<void(const ui::String&)> f) {
        m_onChange = std::move(f);
        return *this;
    }

private:
    ui::PropertyOrValue<std::vector<ui::String>> m_items;
    std::function<void(const ui::String&)> m_onChange;

    ui::AnyComponent render() const override {
        using namespace ui;
        const auto& s = state();
        return If(s.expanded).then(
            "Expanded"
        ).otherwise(
            Button()
        );
    }


};
*/

int main(){

    using namespace ui;

    // auto str = Property<String>("Dagnabbit darn drat!");
    /*
    AnyComponent comp = UseFont(&getFont()).with(
        PulldownMenu(std::vector<String>{"One", "Two", "Three", "Four", "Five"})
            .onChange([](const ui::String& s){ std::cout << "You chose " << s.toAnsiString() << "\n"; })
    );
    */
    
    auto vec = Property{std::vector<int>{0, 1, 2, 3, 4, 5, 6}};
    
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
            }),
            "There are ",
            TextComponent(DerivedProperty<String, std::vector<int>>(
                [](const ListOfEdits<int>& v){ return ui::String(std::to_string(v.newValue().size())); },
                vec
            )),
            " things."
        )
    );
    

    auto root = Root::create<dom::FlowContainer>(std::move(comp));

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