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

    auto randEng = std::default_random_engine{std::random_device{}()};
    auto dist = std::uniform_real_distribution<float>{0.0f, 1.0f};

    using MyComponent = MixedComponent<Clickable, Boxy, Resizable, Scrollable>;
    using Action = MyComponent::Action;

    auto bgColor = Property<Color>{0xFFFF00FF};

    auto inFocus = Property<bool>{false};

    auto pos = Property{vec2{0.0f, 0.0f}};

    AnyComponent comp = UseFont(&getFont()).with(List(
        "Before  ",
        MyComponent{}
            .width(100.0f)
            .height(100.0f)
            .onLeftClick([&](int i, Action action){
                auto c = bgColor.getOnce();
                c.setHue(dist(randEng));
                bgColor.set(c);
                return true;
            })
            .onScroll([&](vec2 delta) {
                auto c = bgColor.getOnce();
                c.setSaturation(c.saturation() + delta.x * 0.05f);
                c.setLightness(c.lightness() + delta.y * 0.05f);
                bgColor.set(c);
                return true;
            })
            .backgroundColor(bgColor)
            .borderColor(0xFF)
            .borderThickness(5.0f)
            .borderRadius(10.0f),
        "  After",
        MixedComponent<Focusable, Boxy, Resizable, KeyPressable>{}
            .onGainFocus([&](){
                inFocus.set(true);
            })
            .onLoseFocus([&](){
                inFocus.set(false);
            })
            .size(vec2{100.0f, 20.0f})
            .backgroundColor(inFocus.map([](bool b) -> Color {
                return b ? 0xFF0000FF : 0x0000FFFF;
            })),
        FreeContainer(
            MixedComponent<Clickable, Boxy, Resizable, Positionable, Draggable, KeyPressable, HitTestable>{}
                .backgroundColor(0xFF0000FF)
                .borderColor(0x440000FF)
                .borderThickness(2.0f)
                .borderRadius(10.0f)
                .width(20.0f)
                .height(20.0f)
                .hitTest([](vec2 p) {
                    const auto d = p - vec2{10.0f, 10.0f};
                    return (d.x * d.x + d.y * d.y <= 100.0f);
                })
                .position(pos)
                .onKeyDown([&](Key k){
                    if (k == Key::Return){
                        pos.set({
                            dist(randEng) * 100.0f,
                            dist(randEng) * 100.0f
                        });
                        return true;
                    }
                    return false;
                })
                .onLeftClick([](int, auto action) {
                    action.startDrag();
                    return true;
                })
                .onLeftRelease([](auto action) {
                    action.stopDrag();
                    return true;
                })
        )
    ));
    
    /*
    auto num = Property<int>{0};

    AnyComponent comp = UseFont(&getFont()).with(List(
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

    /*
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
    */

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