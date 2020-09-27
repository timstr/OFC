#include <OFC/UI.hpp>

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
        font.loadFromFile(fonts_path + "/mononoki-Regular.ttf");
        loaded = true;
    }
    return font;
}

template<typename T>
ofc::String make_string(const T& t) {
    return std::to_string(t);
}

// TODO: persistent additional state
// Supposing some model is being used to generate the UI (such as a flo::Network containing various properties),
// there may be additional state stored in the components using that model which is not intrinsically part of the
// model (such as on-screen position of an object, colour of some widget, other user preferences, etc).
// The additional state on its own is simple, each stateful component can simply store what it wants to in its state.
// The only tricky part is serialization and deserialization.
// Design option 1:
// every Value<T> allows attaching state that is associated with a specific instance of a component.
// (-) this would bloat every Value
// (-) mapping between component states and model state would be challening and error-prone
// (-) some Value<T> values are not directly tied to the model (such as derived properties)
// (-) model state might have too many responsibilities
// Design option 2:
// model state and all (or just some) UI state is serialized separately. UI state has some kind of deterministic and
// error-checking structure for doing this safely.
// (+) model structure and Value<T> interface can stay the same (serialization will still be needed either way)
// (+) component hierarchy is already well-structured, model state can probably be managed by recursing through
//     the tree
// (+) far more separation of concerns
// Example serialization:
// 1. model only is serialized
// 2. UI state is serialized by recursing through all instantiated components in order
// Example deserialization:
// 1. model only is deserialized
// 2. UI is instantiated (either using default state)
// 2. UI state is deserialized by recursing through all instantiated components in order (the component structure must
//    match that of the serialized state, but this should be easy enough given the functional style of the component API)
// The above workflows could be enabled simply by adding something to the Component interface for serializing/deserializing
// the component's state, if any exists. A couple virtual methods could achieve this simply.


struct CoolState {
    ofc::Value<ofc::vec2> position;
};

void serialize(ofc::Serializer& s, const CoolState& cs) {
    auto p = cs.position.getOnce();
    s.f32(p.x).f32(p.y);
}

void deserialize(ofc::Deserializer& d, CoolState& cs) {
    cs.position.set(ofc::vec2{d.f32(), d.f32()});
}

class CoolComponent : public ofc::ui::StatefulComponent<CoolState, ofc::ui::Persistent> {
public:
    CoolComponent()
        : StatefulComponent(CoolState{ ofc::Value{ofc::vec2{0.0f, 0.0f}} }) {

    }

private:
    ofc::ui::AnyComponent render() const override {
        using namespace ofc;
        using namespace ofc::ui;
        return MixedContainerComponent<FreeContainerBase, Boxy, Resizable>{}
            .backgroundColor(0xFF0000FF)
            .size(vec2{500.0f, 500.0f})
            .borderRadius(5.0f)
            .containing("Hello, world!");
    }
};

// TODO: animations
// the way to enable animations should be as part of StatefulComponent:
// - animations can modify some part of a component's state
// - This could completely supercede the existing transitions api
// - how to do???

// TODO: make a large graph-like data structure and create a UI component for it to test how well this whole library actually works

// TODO: (re)implement helpers like:
// - pulldown menu (see above)
// - text fields
// - checkbox
// - 

int main(){

    using namespace ofc;
    using namespace ofc::ui;

    // AnyComponent comp = UseFont(&getFont()).with(CoolComponent{});

    auto Box = [](AnyComponent c) -> AnyComponent {
        return MixedContainerComponent<FreeContainerBase, Boxy, Resizable, Clickable>{}
            .sizeForce(vec2{30.0f, 30.0f})
            .backgroundColor(0x66FF66FF)
            .borderColor(0x440000FF)
            .borderThickness(2.5f)
            .containing(
                Center{std::move(c)}
            );
    };

    auto numItems = Value<std::size_t>{1};

    auto items = numItems.map([](std::size_t n){
        std::vector<String> items;
        items.reserve(n);
        for (std::size_t i = 0; i < n; ++i){
            items.push_back(std::to_string(i));
        }
        return items;
    });

    auto someNumber = Value{5.0f};
    auto someBoolean = Value{true};
    auto someItems = Value{std::vector<std::pair<int, String>>{{1, "One"}, {2, "Two"}, {3, "Three"}}};

    AnyComponent comp = UseFont(&getFont()).with(
        MixedContainerComponent<FlowContainerBase, Boxy, Resizable>{}
            .minSize(vec2{500.0f, 500.0f})
            .backgroundColor(0x000040FF)
            .containing(
                MixedContainerComponent<ColumnGridBase, Boxy, Resizable>{RightToLeft, TopToBottom}
                    .backgroundColor(0xFF0044FF)
                    .borderColor(0xFFFF00FF)
                    .borderThickness(5.0f)
                    .borderRadius(5.0f)
                    .containing(
                        Column(Box("A")),
                        Column(Box("A"), Box("B")),
                        Column(Box("A"), Box("B"), Box("C")),
                        Column(Box("D"), Box("E"), Box("F"), Box("G"), Box("H")),
                        Column(Box("I"), Box("J")),
                        Column(ForEach(items).Do([&](const String& s) -> AnyComponent { return Box(s); }))
                    ),
                " ",
                Button("+").onClick([&](){ numItems.set(numItems.getOnce() + 1); }),
                " ",
                Button("-").onClick([&](){ numItems.set(std::max(std::size_t{1}, numItems.getOnce()) - 1); }),
                " ",
                Slider<float>{0.0f, 10.0f, someNumber}.onChange([&](float x){ someNumber.set(x); }),
                CheckBox(someBoolean).onChange([&](bool b){ someBoolean.set(b); }),
                PulldownMenu(someItems)
            )
    );

    /* auto words = Value{std::vector<String>{"Hello", "world"}};

    AnyComponent comp = UseFont(&getFont()).with(
        MixedContainerComponent<FreeContainerBase, Boxy, Resizable, Clickable>{
                Center{HorizontalList{
                    ForEach(words).Do([](const ui::String& s){ return s; })
                }}
            }
            .backgroundColor(0xFF0000FF)
            .minSize(vec2{200.0f, 200.0f})
            .borderRadius(5.0f)
            .onLeftClick([&](int){
                std::cout << "AAAaaa you clicked me\n";
                words.getOnceMut().push_back("eek"); 
                return true;
            })
    ); */

    // TODO: how to access mounted component for serialization?

    /*auto randEng = std::default_random_engine{std::random_device{}()};
    auto dist = std::uniform_real_distribution<float>{0.0f, 1.0f};

    using MyComponent = MixedComponent<Clickable, Boxy, Resizable, Scrollable>;
    using Action = MyComponent::Action;

    auto bgColor = Value<Color>{0xFFFF00FF};

    {
        auto s = Serializer{};

        s.object(bgColor.getOnce());

        auto d = Deserializer{s.dump()};

        bgColor.set(d.object<Color>());
    }

    auto inFocus = Value<bool>{false};

    auto pos = Value{vec2{0.0f, 0.0f}};

    auto dropFieldColor = Value<Color>{0x808080FF};

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
        MixedComponent<Hoverable, Boxy, Resizable, Positionable>{}
            .position(vec2{100.0f, 200.0f})
            .size(vec2{100.0f, 100.0f})
            .backgroundColor(dropFieldColor)
            .borderThickness(1.0f)
            .borderColor(0xFF)
            .onMouseEnter([&]{
                dropFieldColor.set(0x80B080FF);
            })
            .onMouseEnterWith<int>([&](int){
                dropFieldColor.set(0x80FF80FF);
            })
            .onMouseLeave([&] {
                dropFieldColor.set(0x808080FF);
            })
            .onDrop<int>([&](int i) {
                std::cout << "An int was dropped with value " << i << '\n';
                dropFieldColor.set(0x40FF40FF);
                return true;
            }),
        FreeContainer(
            MixedComponent<Clickable, Boxy, Resizable, Positionable, Draggable, KeyPressable, HitTestable>{}
                .backgroundColor(0xFF0000FF)
                .borderColor(0x440000FF)
                .borderThickness(2.0f)
                .borderRadius(25.0f)
                .width(50.0f)
                .height(50.0f)
                .hitTest([](vec2 p) {
                    const auto d = p - vec2{25.0f, 25.0f};
                    return (d.x * d.x + d.y * d.y <= 25.0f * 25.0f);
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
                    action.startDrag<int>(42);
                    return true;
                })
                .onLeftRelease([](auto action) {
                    action.stopDrag();
                    auto v = 999;
                    std::cout << "Dropping an int with value " << v << '\n';
                    action.drop<int>(v);
                    return true;
                })
        )
    ));*/
    
    
    /*
    auto num = Value<int>{0};

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

    
    /*auto items = Value{std::vector{
        "aaa", "bbb", "ccc", "ddd", "eee",
        "fff", "ggg", "hhh", "iii", "jjj",
        "kkk", "lll", "mmm", "nnn", "ooo",
        "ppp", "qqq", "rrr", "sss", "ttt"
    }};

    
    AnyComponent comp = UseFont(&getFont()).with(List(
        MixedContainerComponent<WrapGridBase, Boxy>(TopToBottom, 5, RightToLeft)
            .backgroundColor(0xffb0b0)
            .borderColor(0xff)
            .borderThickness(1.0f)
            .borderRadius(3.0f)
            .containing(
                ForEach(items).Do([](const char* s, const Value<std::size_t>& idx) -> AnyComponent {
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
    auto vec = Value{std::vector<int>{0, 1, 2, 3, 4, 5, 6}};
    auto num = Value<std::size_t>{1000};

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
            Text(vec.map(
                [](const ListOfEdits<int>& v){ return make_string(v.newValue().size()); }
            )),
            " things.",
            " If you added ",
            Text(num.map([](std::size_t n){ return make_string(n); })),
            " then you would have ",
            Text(combine(vec, num).map([](const ListOfEdits<int>& v, std::size_t n) {
                return make_string(v.newValue().size() + n);
            })),
            " things."
        )
    );
    */

    auto root = Root(FreeContainer{}.containing(std::move(comp)));

    /*
    auto condition = Value<bool>{true};

    AnyComponent comp = UseFont(&getFont()).with(
        If(condition).then(
            Button().caption("Click Me!").onClick([&](){ condition.set(false); })
        ).otherwise(
            "Dagnabbit darn drat.."
        )
    );
    auto root = ComponentRoot::create<FreeContainer>(std::move(comp));
    */

    Window& win = Window::create(std::move(root), 600, 400, "Test");

    run();

    return 0;
}