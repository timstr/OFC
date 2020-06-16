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

struct PulldownMenuState {
    ui::Property<std::size_t> currentIndex {0};
    ui::Property<bool> expanded {false};
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

        const auto toggleExpanded = [this]{
            const auto e = state().expanded.getOnce();
            stateMutable().expanded.set(!e);
        };

        const auto indexItems = [](std::size_t i, const ui::ListOfEdits<ui::String>& e) {
            assert(i < e.newValue().size());
            return e.newValue()[i];
        };

        const auto makeItem = [this](const String& str, const std::size_t& i) -> AnyComponent {
            return Button(str).onClick([this, str, i](){
                if (m_onChange){
                    m_onChange(str);
                }
                stateMutable().currentIndex.set(i);
                stateMutable().expanded.set(false);
            });
        };

        const auto& s = state();
        return If(s.expanded).then(
            VerticalList(List(
                Button(
                    combine(s.currentIndex, m_items).map(indexItems)
                ).onClick(toggleExpanded),
                FreeContainer(VerticalList(
                    ForEach(m_items).Do(makeItem)
                ))
            ))
        ).otherwise(
            Button(
                combine(s.currentIndex, m_items).map(indexItems)
            ).onClick(toggleExpanded)
        );
    }


};

template<typename T>
ui::String make_string(const T& t) {
    return std::to_string(t);
}

int main(){

    using namespace ui;

    AnyComponent comp = UseFont(&getFont()).with(
        PulldownMenu(std::vector<String>{"One", "Two", "Three", "Four", "Five"})
            .onChange([](const ui::String& s){ std::cout << "You chose " << s.toAnsiString() << "\n"; })
    );
    

    /*
    auto vec = Property{std::vector<int>{0, 1, 2, 3, 4, 5, 6}};
    auto num = Property<std::size_t>{1000};

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
            TextComponent(vec.map(
                [](const ListOfEdits<int>& v){ return make_string(v.newValue().size()); }
            )),
            " things.",
            " If you added ",
            TextComponent(num.map([](std::size_t n){ return make_string(n); })),
            " then you would have ",
            TextComponent(combine(vec, num).map([](const ListOfEdits<int>& v, std::size_t n) {
                return make_string(v.newValue().size() + n);
            })),
            " things."
        )
    );
    */

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