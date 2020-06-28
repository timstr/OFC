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

    PulldownMenu& onChange(std::function<void(const ui::String&, std::size_t)> f) {
        m_onChange = std::move(f);
        return *this;
    }

    PulldownMenu& onChange(std::function<void(const ui::String&)> f) {
        m_onChange = [f = std::move(f)](const ui::String& s, std::size_t /* index */){
            f(s);
        };
        return *this;
    }

private:
    ui::PropertyOrValue<std::vector<ui::String>> m_items;
    std::function<void(const ui::String&, std::size_t)> m_onChange;

    ui::AnyComponent render() const override {
        using namespace ui;

        const auto toggleExpanded = [this]{
            const auto e = state().expanded.getOnce();
            stateMut().expanded.set(!e);
        };

        const auto indexItems = [](std::size_t i, const ui::ListOfEdits<ui::String>& e) {
            assert(i < e.newValue().size());
            return e.newValue()[i];
        };

        const auto makeItem = [this](const String& str, const Property<std::size_t>& idx) -> AnyComponent {
            return Button(str).onClick([this, str, &idx](){
                if (m_onChange){
                    m_onChange(str, idx.getOnce());
                }
                stateMut().currentIndex.set(idx.getOnce());
                stateMut().expanded.set(false);
            });
        };

        const auto& s = state();

        return VerticalList(List(
            Button(
                combine(s.currentIndex, m_items.view()).map(indexItems)
            ).onClick(toggleExpanded),
            If(s.expanded)
                .then(FreeContainer(VerticalList(
                    ForEach(m_items.view()).Do(makeItem)
                ))
            )
        ));
    }


};

template<typename T>
ui::String make_string(const T& t) {
    return std::to_string(t);
}

int main(){

    using namespace ui;

    auto num = Property<int>{0};

    /*AnyComponent comp = UseFont(&getFont()).with(List(
        PulldownMenu(std::vector<String>{"Zero", "One", "Two", "Three", "Four", "Five"})
            .onChange([&](const ui::String& s, std::size_t i){
                std::cout << "You chose " << s.toAnsiString() << "\n";
                num.set(static_cast<int>(i));
            }),
        " ",
        Switch(num)
            .Case(0, "Zero")
            .Case(1, "One")
            .Case(2, "Two")
            .Default("Something else, idk"),
        ". ",
        If(num.map([](int i){ return i % 2 == 0; }))
            .then("It is even.")
            .otherwise("It is odd.")
    ));*/

    auto items = Property{std::vector{
        "aaa", "bbb", "ccc", "ddd", "eee",
        "fff", "ggg", "hhh", "iii", "jjj",
        "kkk", "lll", "mmm", "nnn", "ooo",
        "ppp", "qqq", "rrr", "sss", "ttt"
    }};

    AnyComponent comp = UseFont(&getFont()).with(List(
        WrapGrid(TopToBottom, 5, RightToLeft).Containing(
            ForEach(items).Do([](const char* s, const Property<std::size_t>& idx) -> AnyComponent {
                return If(idx.map([](std::size_t i){ return i % 2 == 0; }))
                    .then(s)
                    .otherwise(Button(s));
            })
        ),
        Button("Remove one").onClick([&] {
            auto& v = items.getOnceMut();
            if (v.size() > 0){
                v.erase(v.begin());
            }
        })
    ));

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

    auto root = Root::create<FlowContainer>(std::move(comp));

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