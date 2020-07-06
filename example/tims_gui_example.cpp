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

// BEGIN TESTING
#include <GUI/DOM/Control.hpp>
#include <GUI/DOM/BoxElement.hpp>

namespace ui {

    template<typename Derived>
    class Clickable {
    public:
        using ElementBase = ui::dom::Control;

        template<typename Base>
        class ElementMixin : public Base {
        public:
            ElementMixin(Derived& component)
                : Base(component)
                , m_onLeftClick(static_cast<Clickable<Derived>&>(component).m_onLeftClick)
                , m_onLeftRelease(static_cast<Clickable<Derived>&>(component).m_onLeftRelease) {

                static_assert(std::is_base_of_v<dom::Control, Base>, "Base class must derive from ui::dom::Control");
            }

            bool onLeftClick(int i) override final {
                if (m_onLeftClick) {
                    return m_onLeftClick(i);
                }
                return false;
            }

            void onLeftRelease() override final {
                if (m_onLeftRelease) {
                    m_onLeftRelease();
                }
            }

            std::function<bool(int)>& m_onLeftClick;
            std::function<void()>& m_onLeftRelease;
        };

        Derived& onLeftClick(std::function<bool(int)> f) {
            m_onLeftClick = std::move(f);
            return static_cast<Derived&>(*this);
        }
        Derived& onLeftRelease(std::function<void()> f) {
            m_onLeftRelease = std::move(f);
            return static_cast<Derived&>(*this);
        }

        // TODO: middle/right

    private:
        std::function<bool(int)> m_onLeftClick;
        std::function<void()> m_onLeftRelease;
    };

    template<typename Derived>
    class Scrollable {
        // TODO
    };

    // TODO: Hoverable
    // TODO: Keyboard something something?

    template<typename Derived>
    class Draggable {
        // TODO
    };

    template<typename Derived>
    class Boxy {
    public:
        // TODO: do something about all these darn CRTP down-casts
        Boxy()
            : m_backgroundColorObserver(
                static_cast<Derived*>(this),
                static_cast<void (Derived::*)(const Color&)>(&Boxy::onUpdateBackgroundColor)
            )
            , m_borderColorObserver(
                static_cast<Derived*>(this),
                static_cast<void (Derived::*)(const Color&)>(&Boxy::onUpdateBorderColor)
            )
            , m_borderThicknessObserver(
                static_cast<Derived*>(this),
                static_cast<void (Derived::*)(float)>(&Boxy::onUpdateBorderThickness)
            )
            , m_borderRadiusObserver(
                static_cast<Derived*>(this),
                static_cast<void (Derived::*)(float)>(&Boxy::onUpdateBorderRadius)
            ) {

        }

        using ElementBase = dom::BoxElement;

        template<typename Base>
        class ElementMixin : public Base {
        public:
            ElementMixin(Derived& component)
                : Base(component) {
                // HACK
                // TESTING
                setSize({100.0, 100.0}, true);


                setBackgroundColor(component.m_backgroundColorObserver.getValueOnce());
                setBorderColor(component.m_borderColorObserver.getValueOnce());
                setBorderThickness(component.m_borderThicknessObserver.getValueOnce());
                setBorderRadius(component.m_borderRadiusObserver.getValueOnce());
            }
        };

        Derived& backgroundColor(PropertyOrValue<Color> pv) {
            m_backgroundColorObserver.assign(std::move(pv));
            return static_cast<Derived&>(*this);
        }

        Derived& borderColor(PropertyOrValue<Color> pv) {
            m_borderColorObserver.assign(std::move(pv));
            return static_cast<Derived&>(*this);
        }

        Derived& borderThickness(PropertyOrValue<float> pv) {
            m_borderThicknessObserver.assign(std::move(pv));
            return static_cast<Derived&>(*this);
        }

        Derived& borderRadius(PropertyOrValue<float> pv) {
            m_borderRadiusObserver.assign(std::move(pv));
            return static_cast<Derived&>(*this);
        }


    private:
        Observer<Color> m_backgroundColorObserver;
        Observer<Color> m_borderColorObserver;
        Observer<float> m_borderThicknessObserver;
        Observer<float> m_borderRadiusObserver;

        void onUpdateBackgroundColor(const Color& c) {
            auto e = static_cast<Derived*>(this)->element();
            assert(e);
            e->setBackgroundColor(c);
        }

        void onUpdateBorderColor(const Color& c) {
            auto e = static_cast<Derived*>(this)->element();
            assert(e);
            e->setBorderColor(c);
        }

        void onUpdateBorderThickness(float v) {
            auto e = static_cast<Derived*>(this)->element();
            assert(e);
            e->setBorderThickness(v);
        }

        void onUpdateBorderRadius(float v) {
            auto e = static_cast<Derived*>(this)->element();
            assert(e);
            e->setBorderRadius(v);
        }
    };


    // Adapted from https://stackoverflow.com/a/57528226/5023438
    template<typename... Ts>
    struct RemoveDuplicatesImpl {
        // Type is deliberately not defined.
        // This base case allows for zero types to make the syntax easier on the
        // eyes, but using this template with zero arguments and attempting
        // to access Type will result in a substitution failure
    };

    template<typename T, typename... Ts>
    struct RemoveDuplicatesImpl<T, Ts...> {
        using Type = T;
    };

    template<typename... Ts, typename U, typename... Us>
    struct RemoveDuplicatesImpl<std::tuple<Ts...>, U, Us...>
        : std::conditional_t<
            (std::is_same_v<U, Ts> || ...), // Does U match anything in TS?
            RemoveDuplicatesImpl<std::tuple<Ts...>, Us...>, // if yes, recurse but don't add the type to the tuple
            RemoveDuplicatesImpl<std::tuple<Ts..., U>, Us...> // if no, recurse and add the type to the tuple
        > {

    };

    template<typename... Ts>
    using RemoveDuplicates = typename RemoveDuplicatesImpl<std::tuple<>, Ts...>::Type;

    template<typename... Bases>
    class InheritHorizontal : public Bases... {

    };

    template<typename T>
    class InheritHorizontalTuple;

    template<typename... Bases>
    class InheritHorizontalTuple<std::tuple<Bases...>> : public InheritHorizontal<Bases...> {

    };

    template<typename... ElementBases>
    class CommonElementBase
        : public InheritHorizontalTuple<RemoveDuplicates<ElementBases...>> {
    public:
        template<typename Derived>
        CommonElementBase(Derived& /* unused */) {
            static_assert(
                (std::is_base_of_v<dom::Element, ElementBases> && ...),
                "All element bases must derive from ui::dom::Element"
            );
        }
    };

    template<typename BaseBase, template<typename> typename... Mixins>
    class InheritVertical; // Deliberately undefined

    template<typename BaseBase>
    class InheritVertical<BaseBase> : public BaseBase {
    public:
        template<typename... Args>
        InheritVertical(Args&&... args)
            : BaseBase(std::forward<Args>(args)...) {
            
        }
    };

    template<typename BaseBase, template<typename> typename Mixin, template<typename> typename... Rest>
    class InheritVertical<BaseBase, Mixin, Rest...>
        : public Mixin<InheritVertical<BaseBase, Rest...>> {
    public:
        template<typename... Args>
        InheritVertical(Args&&... args)
            : Mixin<InheritVertical<BaseBase, Rest...>>(std::forward<Args>(args)...) {
            
        }
    };

    template<typename Derived, template<typename> typename... Features>
    using MixedElement = InheritVertical<
        CommonElementBase<typename Features<Derived>::ElementBase...>,
        Features<Derived>::template ElementMixin...
    >;

    // TODO: add support for containers using existing container components
    template<template<typename> typename... Features>
    class MixedComponent
        : public SimpleComponent<
            MixedElement<MixedComponent<Features...>, Features...>
        >
        , public Features<MixedComponent<Features...>>... {

    public:
        using ElementType = MixedElement<MixedComponent<Features...>, Features...>;

        // NOTE: various named-parameter-idiom methods are inherited from CRTP 'Features' base classes

    private:
        std::unique_ptr<ElementType> createElement() override final {
            static_assert(
                std::is_constructible_v<ElementType, MixedComponent<Features...>&>,
                "ElementType must be constructible from a reference to the augmented component type"
            );
            return std::make_unique<ElementType>(*this);
        }
    };

} // namespace ui
// END TESTING


int main(){

    using namespace ui;

    AnyComponent comp = UseFont(&getFont()).with(List(
        "Before  ",
        MixedComponent<Clickable, Boxy>{}
            .onLeftClick([](int i){
                std::cout << "clicked " << i << " time(s)\n";
                return true;
            }).
            onLeftRelease([] {
                std::cout << "released\n";
            })
            .backgroundColor(0xFFFF00FF)
            .borderColor(0xFF)
            .borderThickness(5.0f)
            .borderRadius(10.0f),
        "  After"
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