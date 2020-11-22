#include <OFC/UI.hpp>

#include <iostream>
#include <random>
#include <sstream>

std::random_device randdev;
std::mt19937 randeng { randdev() };

const sf::Font& getFont() {
    static sf::Font font;
    static bool loaded;
    if (!loaded) {
        font.loadFromFile("fonts/mononoki-Regular.ttf");
        loaded = true;
    }
    return font;
}

template<typename T>
ofc::String make_string(const T& t) {
    return std::to_string(t);
}

// TODO: actually use C++20 using /std:latest

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



// TODO: animations
// the way to enable animations should be as part of StatefulComponent:
// - animations can modify some part of a component's state
// - This could completely supercede the existing transitions api
// - how to do???

// TODO: make a large graph-like data structure and create a UI component for it to test how well this whole library actually works

using namespace ofc;
using namespace ofc::ui;

class Node {
public:
    Node() noexcept = default;
    virtual ~Node() noexcept = default;

    void connect(Node* other) {
        auto& mine = m_connections.getOnceMut();
        auto& yours = other->m_connections.getOnceMut();
        assert(count(begin(mine), end(mine), other) == 0);
        assert(count(begin(yours), end(yours), this) == 0);
        mine.push_back(other);
        yours.push_back(this);
    }

    void disconnect(Node* other) {
        auto& mine = m_connections.getOnceMut();
        auto& yours = other->m_connections.getOnceMut();
        assert(count(begin(mine), end(mine), other) == 1);
        assert(count(begin(yours), end(yours), this) == 1);
        {
            const auto it = find(begin(mine), end(mine), other);
            assert(it != end(mine));
            mine.erase(it);
        }
        {
            const auto it = find(begin(yours), end(yours), this);
            assert(it != end(yours));
            yours.erase(it);
        }
    }

    const Value<std::vector<Node*>>& connections() const noexcept {
        return m_connections;
    }

private:
    Value<std::vector<Node*>> m_connections;
};

class IntNode : public Node {
public:
    IntNode(int data)
        : m_data(data) {
    
    }

    const Value<int>& data() const noexcept {
        return m_data;
    }

private:
    Value<int> m_data;
};

class StringNode : public Node {
public:
    StringNode(const String& data)
        : m_data(data) {
    
    }

    const Value<String>& data() const noexcept {
        return m_data;
    }

private:
    Value<String> m_data;
};



class Graph {
public:
    void add(std::unique_ptr<Node> n) {
        m_nodes.getOnceMut().push_back(std::move(n));
    }

    void remove(Node* n) {
        auto sameNode = [&](const std::unique_ptr<Node>& up) {
            return up.get() == n;
        };
        auto& theNodes = m_nodes.getOnceMut();
        assert(count_if(begin(theNodes), end(theNodes), sameNode) == 1);
        auto it = find_if(begin(theNodes), end(theNodes), sameNode);
        assert(it != end(theNodes));
        theNodes.erase(it);
    }

    Valuelike<std::vector<const Node*>> nodes() const {
        return m_nodes.map([](const ListOfEdits<const Node*>& v){
            return v.newValue();
        });
    }

private:
    Value<std::vector<std::unique_ptr<Node>>> m_nodes;
};

class GraphUI : public PureComponent {
public:
    GraphUI(const Graph* graph)
        : m_graph(graph) {
    
    }

private:
    AnyComponent render() const override final {
        
    }

    const Graph* const m_graph;
};

int main(){

    auto Box = [](const String& s) -> AnyComponent {
        return MixedContainerComponent<FreeContainerBase, Boxy, Resizable, Clickable>{}
            .sizeForce(vec2{30.0f, 30.0f})
            .backgroundColor(0x66FF66FF)
            .borderColor(0x440000FF)
            .borderThickness(2.5f)
            .containing(
                Center{Text(s)}
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
    auto currentItem = Value{static_cast<std::size_t>(-1)};
    auto toggleState = Value{false};
    auto theString = Value{String{"Enter text here..."}};
    auto theDouble = Value{0.001};
    auto theInt = Value{99};

    AnyComponent comp = UseFont(&getFont()).with(
        MixedContainerComponent<FlowContainerBase, Boxy, Resizable>{}
            .minSize(vec2{500.0f, 500.0f})
            .backgroundColor(0x8080FFFF)
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
                Text(" "),
                Button("+").onClick([&](){ numItems.set(numItems.getOnce() + 1); }),
                Text(" "),
                Button("-").onClick([&](){ numItems.set(std::max(std::size_t{1}, numItems.getOnce()) - 1); }),
                Text(" "),
                Slider<float>{0.0f, 10.0f, someNumber}.onChange([&](float x){ someNumber.set(x); }),
                CheckBox(someBoolean).onChange([&](bool b){ someBoolean.set(b); }),
                PulldownMenu(someItems, currentItem).onChange([&](int, std::size_t i){ currentItem.set(i); }),
                UseCommands{}
                    .add(Key::Space, ModifierKeys::Ctrl, [&](){ toggleState.set(!toggleState.getOnce()); })
                    .with(
                        Toggle("Off", "On", toggleState).onChange([&](bool b){ toggleState.set(b); })
                    ),
                TextField{theString}.onSubmit([&](const String& s){ theString.set(s); }),
                Span(theString.map([](const String& s){ return "The string is \"" + s + "\""; })),
                NumberTextField{theInt}.onSubmit([&](int i){ theInt.set(i); }),
                Span(theInt.map([](int i) -> String { return "The int is \"" + std::to_string(i) + "\""; }))
            )
    );


    auto root = Root(FreeContainer{}.containing(std::move(comp)));

    Window& win = Window::create(std::move(root), 600, 400, "Test");

    run();

    return 0;
}