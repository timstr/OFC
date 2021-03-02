#include <OFC/UI.hpp>

#include <cassert>
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
using namespace std::string_literals;

struct Pinned {
    Pinned() noexcept = default;
    ~Pinned() noexcept = default;

    Pinned(Pinned&&) = delete;
    Pinned(const Pinned&) = delete;

    Pinned& operator=(Pinned&&) = delete;
    Pinned& operator=(const Pinned&) = delete;
};

class Graph;

class Node {
public:
    Node() noexcept
        : m_inputs(defaultConstruct)
        , m_outputs(defaultConstruct)
        , m_parentGraph(nullptr) {
        
    }

    virtual ~Node() noexcept {
        assert(m_parentGraph == nullptr);
        assert(m_inputs.getOnce().size() == 0);
        assert(m_outputs.getOnce().size() == 0);
    }

    void addInput(Node* other) {
        assert(m_parentGraph);
        assert(other->m_parentGraph == m_parentGraph);
        auto& mine = m_inputs.getOnceMut();
        auto& yours = other->m_outputs.getOnceMut();
        assert(count(begin(mine), end(mine), other) == 0);
        assert(count(begin(yours), end(yours), this) == 0);
        mine.push_back(other);
        yours.push_back(this);
    }

    void removeInput(Node* other) {
        assert(m_parentGraph);
        assert(other->m_parentGraph == m_parentGraph);
        auto& mine = m_inputs.getOnceMut();
        auto& yours = other->m_outputs.getOnceMut();
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

    void removeAllConnections() {
        assert(m_parentGraph);
        while (m_inputs.getOnce().size() > 0) {
            const auto i = m_inputs.getOnce().front();
            assert(i->m_parentGraph == m_parentGraph);
            removeInput(i);
        }
        while (m_outputs.getOnce().size() > 0) {
            auto o = m_outputs.getOnce().front();
            assert(o);
            assert(o->m_parentGraph == m_parentGraph);
            o->removeInput(this);
        }
    }

    const Value<std::vector<Node*>>& inputs() const noexcept {
        return m_inputs;
    }

    const Value<std::vector<Node*>>& outputs() const noexcept {
        return m_outputs;
    }

    virtual const std::string& getType() const noexcept = 0;

    Graph* getGraph() noexcept {
        return m_parentGraph;
    }

    const Graph* getGraph() const noexcept {
        return m_parentGraph;
    }

private:
    Value<std::vector<Node*>> m_inputs;
    Value<std::vector<Node*>> m_outputs;
    Graph* m_parentGraph;

    friend Graph;
};

class IntegerNode : public Node {
public:
    static inline const std::string Type = "Integer"s;

    IntegerNode(int data)
        : m_data(data) {
    
    }

    void setData(int i) {
        m_data.set(i);
    }

    const Value<int>& data() const noexcept {
        return m_data;
    }

    const std::string& getType() const noexcept override final {
        return Type;
    }

private:
    Value<int> m_data;
};

class StringNode : public Node {
public:
    static inline const std::string Type = "String"s;

    StringNode(const String& data)
        : m_data(data) {
    
    }

    void setData(const String& s) {
        m_data.set(s);
    }

    const Value<String>& data() const noexcept {
        return m_data;
    }

    const std::string& getType() const noexcept override final {
        return Type;
    }

private:
    Value<String> m_data;
};

class BooleanNode : public Node {
public:
    static inline const std::string Type = "Boolean"s;

    BooleanNode(bool data)
        : m_data(data) {
    
    }

    void setData(bool b) {
        m_data.set(b);
    }

    const Value<bool>& data() const noexcept {
        return m_data;
    }

    const std::string& getType() const noexcept override final {
        return Type;
    }

private:
    Value<bool> m_data;
};



class Graph {
public:
    Graph()
        : m_nodes(defaultConstruct) {

    }

    ~Graph() noexcept {
        for (auto& np : m_nodes.getOnce()) {
            assert(np->m_parentGraph == this);
            np->removeAllConnections();
        }
        for (auto& np : m_nodes.getOnce()) {
            assert(np->m_parentGraph == this);
            np->m_parentGraph = nullptr;
        }
    }

    void adopt(std::unique_ptr<Node> n) {
        assert(n->m_parentGraph == nullptr);
        n->m_parentGraph = this;
        m_nodes.getOnceMut().push_back(std::move(n));
    }

    template<typename T, typename... Args>
    T& add(Args&&... args) {
        auto up = std::make_unique<T>(std::forward<Args>(args)...);
        assert(up->m_parentGraph == nullptr);
        auto& r = *up;
        adopt(std::move(up));
        assert(r.m_parentGraph == this);
        return r;
    }

    std::unique_ptr<Node> release(Node* n) {
        assert(n);
        assert(n->m_parentGraph == this);
        auto sameNode = [&](const std::unique_ptr<Node>& up) {
            return up.get() == n;
        };
        auto& theNodes = m_nodes.getOnceMut();
        assert(count_if(begin(theNodes), end(theNodes), sameNode) == 1);
        auto it = find_if(begin(theNodes), end(theNodes), sameNode);
        assert(it != end(theNodes));

        n->removeAllConnections();

        auto ret = std::move(*it);
        theNodes.erase(it);

        assert(ret->m_parentGraph == this);
        ret->m_parentGraph = nullptr;
        return ret;
    }

    void remove(Node* n) {
        auto p = release(n);
        (void)p;
    }

    Value<std::vector<Node*>> nodes() const {
        return m_nodes.vectorMap([](const std::unique_ptr<Node>& p){
            return p.get();
        });
    }

    using Connection = std::pair<Node*, Node*>;

    Value<std::vector<Connection>> connections() const {

        const auto perNodeConnections = [](Node* n){
            return n->outputs().vectorMap([&](Node* nn){
                assert(n != nn);
                return std::make_pair(n, nn);
            });
        };

        const auto combineConnections = [](std::vector<Connection> acc, const std::vector<Connection>& v) {
            for (const auto& c : v) {
                if (find(begin(acc), end(acc), c) == end(acc)) {
                    acc.push_back(c);
                }
            }
            return acc;
        };

        return nodes().reduce<std::vector<Connection>>(
            std::vector<Connection>{},
            perNodeConnections,
            combineConnections
        );
    }

private:
    Value<std::vector<std::unique_ptr<Node>>> m_nodes;
};

AnyComponent IntNodeUI(IntegerNode* n) {
    return List{
        Text(n->data().map([](int i) -> String {
            return "Integer: " + std::to_string(i);
        })),
        NumberTextField{n->data()}
            .onSubmit([n](int i){
                n->setData(i);
            })
    };
}

AnyComponent StringNodeUI(StringNode* n) {
    return List{
        Text(n->data().map([](const String& s) -> String {
            return "String: \"" + s + "\"";
        })),
        TextField{n->data()}
            .onSubmit([n](const String& s){
                n->setData(s);
            })
    };
}

AnyComponent BooleanNodeUI(BooleanNode* n) {
    using namespace std::literals;
    return List{
        Text(n->data().map([](bool b) -> String {
            return "Boolean: \""s + (b ? "True" : "False") + "\""s;
        })),
        Toggle("False", "True", n->data())
            .onChange([n](bool b){
                n->setData(b);
            })
    };
}

using NodeUICreator = std::function<AnyComponent(Node*)>;

template<typename T, typename F>
NodeUICreator makeNodeUICreator(F&& f) {
    return [f = std::forward<F>(f)](Node* n){
        assert(n);
        static_assert(std::is_base_of_v<Node, T>);
        auto tn = dynamic_cast<T*>(n);
        assert(tn);
        static_assert(std::is_invocable_v<F, T*>);
        return f(tn);
    };
}

AnyComponent MakeNodeUI(Node* n) {
    static auto mapping = std::map<std::string, NodeUICreator>{
        {IntegerNode::Type, makeNodeUICreator<IntegerNode>(IntNodeUI)},
        {StringNode::Type, makeNodeUICreator<StringNode>(StringNodeUI)},
        {BooleanNode::Type, makeNodeUICreator<BooleanNode>(BooleanNodeUI)}
    };

    auto it = mapping.find(n->getType());
    assert(it != end(mapping));
    return it->second(n);
}

class NodeUI : public PureComponent {
public:
    NodeUI(Node* node, Value<vec2> position) 
        : m_node(node)
        , m_position(std::move(position)) {
    
    }

    NodeUI&& onMove(std::function<void(vec2)> f) {
        m_onMove = std::move(f);
        return std::move(*this);
    }

    NodeUI&& onMoveInput(std::function<void(vec2)> f) {
        m_onMoveInput = std::move(f);
        return std::move(*this);
    }

    NodeUI&& onMoveOutput(std::function<void(vec2)> f) {
        m_onMoveOutput = std::move(f);
        return std::move(*this);
    }

private:
    AnyComponent inputPeg() const {
        return MixedContainerComponent<FreeContainerBase, Boxy, Positionable>{}
            .borderRadius(15.0f)
            .backgroundColor(0xF4FF7FFF)
            .borderColor(0xFF)
            .borderThickness(2.0f)
            .onChangeGlobalPosition([&](vec2 v){
                if (m_onMoveInput) {
                    m_onMoveInput(v);
                }
            })
            .containing(
                Center(Text("In"))
            );
    }

    AnyComponent outputPeg() const {
        return MixedContainerComponent<FreeContainerBase, Boxy, Positionable>{}
            .borderRadius(15.0f)
            .backgroundColor(0xF4FF7FFF)
            .borderColor(0xFF)
            .borderThickness(2.0f)
            .onChangeGlobalPosition([&](vec2 v){
                if (m_onMoveOutput) {
                    m_onMoveOutput(v);
                }
            })
            .containing(
                Center(Text("Out"))
            );
    }

    AnyComponent body() const {
        return MixedContainerComponent<VerticalListBase, Boxy, Positionable, Resizable>{}
            .position(m_position)
            .minSize(vec2{50.0f, 50.0f})
            .backgroundColor(0xFFBB99FF)
            .borderColor(0xFF) 
            .borderRadius(10.0f)
            .borderThickness(2.0f)
            .containing(
                List{
                    Expand{HorizontalList{LeftToRight, true}.containing(
                        AlignLeft{Text{"Node"}},
                        Weight{0.0f, Button{"X"}.onClick([this]{
                            m_node->getGraph()->remove(m_node);
                        })}
                    )},
                    MakeNodeUI(m_node)
                }
            );
    }

    AnyComponent render() const override final {
        return MixedContainerComponent<HorizontalListBase, Clickable, Draggable, Positionable>{}
            .onLeftClick([](int, ModifierKeys, auto action){
                action.startDrag();
                return true;
            })
            .onLeftRelease([](auto action){
                action.stopDrag();
            })
            .onChangeGlobalPosition([this](vec2 v){
                if (m_onMove){
                    m_onMove(v);
                }
            })
            .containing(
                CenterVertically{inputPeg()},
                body(),
                CenterVertically{outputPeg()}
            );
    }

    Node* const m_node;
    Value<vec2> m_position;
    std::function<void(vec2)> m_onMove;
    std::function<void(vec2)> m_onMoveInput;
    std::function<void(vec2)> m_onMoveOutput;
};

class WireUI : public PureComponent {
public:
    WireUI(const Node* source, const Node* target)
        : m_source(source)
        , m_target(target) {
    
    }

    WireUI&& sourcePosition(Value<vec2> v) {
        m_sourcePosition = std::move(v);
        return std::move(*this);
    }

    WireUI&& targetPosition(Value<vec2> v) {
        m_targetPosition = std::move(v);
        return std::move(*this);
    }

private:
    const Node* m_source;
    const Node* m_target;

    Value<vec2> m_sourcePosition;
    Value<vec2> m_targetPosition;

    AnyComponent render() const override final {
        return List(
            MixedComponent<Positionable, Resizable, Boxy>{}
                .position(m_sourcePosition)
                .size(vec2{20.0f, 20.0f})
                .borderColor(0xFF)
                .borderRadius(10.0f)
                .backgroundColor(0xBBBB44)
        );
    }
};

class GraphUI : public PureComponent {
public:
    GraphUI(Graph* graph)
        : m_graph(graph)
        , m_nodePositions(graph->nodes().vectorMap([](Node* n){
            return NodePosition{n, vec2{0.0f, 0.0f}};
        }))
        , m_inputPegPositions(graph->nodes().vectorMap([](Node* n){
            return PegPosition{n, vec2{0.0f, 0.0f}};
        }))
        , m_outputPegPositions(graph->nodes().vectorMap([](Node* n){
            return PegPosition{n, vec2{0.0f, 0.0f}};
        })) {

    }

private:
    AnyComponent description() const {
        auto numConnections = m_graph->connections().map([](const ListOfEdits<Graph::Connection>& loe){
            return loe.newValue().size();
        });
        return Text(combine(m_nodePositions, std::move(numConnections)).map([](const ListOfEdits<NodePosition>& loe, std::size_t n) -> String {
            return "There are " + std::to_string(loe.newValue().size()) + " nodes and " + std::to_string(n) + " connections";
        }));
    }

    // Source node & position, target node & position
    using PositionedConnection = std::tuple<Node*, Value<vec2>, Node*, Value<vec2>>;

    Value<std::vector<PositionedConnection>> positionedConnections() const {
        return combine(m_inputPegPositions, m_outputPegPositions, m_graph->connections())
            .map([](
                const ListOfEdits<NodePosition>& ipp,
                const ListOfEdits<NodePosition>& opp,
                const ListOfEdits<Graph::Connection>& conns
            ){
                const auto& is = ipp.newValue();
                const auto& os = opp.newValue();
                const auto& cs = conns.newValue();
                std::vector<PositionedConnection> ret;
                ret.reserve(cs.size());
                for (const auto& c : cs) {
                    auto ii = find_if(
                        begin(is),
                        end(is),
                        [&](const NodePosition& np){ 
                            return np.first == c.second;
                        }
                    );
                    assert(ii != end(is));

                    auto oi = find_if(
                        begin(os),
                        end(os),
                        [&](const NodePosition& np){ 
                            return np.first == c.first;
                        }
                    );
                    assert(oi != end(os));

                    ret.push_back(PositionedConnection{
                        c.first,
                        oi->second,
                        c.second,
                        ii->second
                    });
                }
                return ret;
            });
    }

    AnyComponent render() const override final {
        auto pc = positionedConnections();


        auto va = pc.reduce(
            std::vector<sf::Vertex>{},
            [](const PositionedConnection& p) -> Value<std::pair<vec2, vec2>> {
                return combine(std::get<1>(p), std::get<3>(p))
                    .map([](const vec2& p1, const vec2& p2){
                        return std::make_pair(p1, p2);
                    });
            },
            [](std::vector<sf::Vertex> acc, const std::pair<vec2, vec2>& p1p2){
                acc.push_back(sf::Vertex{p1p2.first, sf::Color{0xFF}});
                acc.push_back(sf::Vertex{p1p2.second, sf::Color{0xFF}});
                return acc;
            }
        );

        return List(
            HorizontalList{}.containing(
                Button("+")
                    .onClick([this](){
                        m_graph->add<StringNode>("...");
                    }),
                description()
            ),
            ForEach(m_nodePositions).Do([this](const NodePosition& np, const Value<std::size_t>& idx) -> AnyComponent {
                return NodeUI{np.first, np.second}
                    .onMove([this, &idx](vec2 v){
                        auto& nps = this->m_nodePositions.getOnce();
                        auto i = idx.getOnce();
                        assert(i < nps.size());
                        nps[i].second.makeMutable().set(v);
                    })
                    .onMoveInput([this, &idx](vec2 v){
                        auto& ips = this->m_inputPegPositions.getOnce();
                        auto i = idx.getOnce();
                        assert(i < ips.size());
                        ips[i].second.makeMutable().set(v);
                    })
                    .onMoveOutput([this, &idx](vec2 v){
                        auto& ips = this->m_outputPegPositions.getOnce();
                        auto i = idx.getOnce();
                        assert(i < ips.size());
                        ips[i].second.makeMutable().set(v);
                    });
            }),
            VertexArray{}
                .vertices(va)
                .primitiveType(sf::Lines),
            ForEach(pc).Do([this](const PositionedConnection& pp, const Value<std::size_t>& idx) -> AnyComponent {
                const auto& [sn, op, tn, ip] = pp;
                const auto makePeg = []{
                    return MixedContainerComponent<FreeContainerBase, Positionable, Resizable, Boxy>{}
                        .size(vec2{20.0f, 20.0f})
                        .borderRadius(10.0f)
                        .borderThickness(2.0f)
                        .borderColor(0xff)
                        .backgroundColor(0x4444bbff);
                };

                return List(
                    makePeg()
                        .position(op)
                        .containing(Center{Text("src")}),
                    makePeg()
                        .position(ip)
                        .containing(Center{Text("dst")})
                );
            })
        );
        // TODO: render connections
        // have: all pairs of connected nodes
        // need: the spatial locations of the input and output pegs of those nodes' UIs
        //       -> how can do???
        //       -> at every point in time, connection endpoints should line up exactly
        //          with the input and output pegs
        //       -> depends on:
        //           - node UI position
        //           - peg position within node UI (which potentially depends on
        //             everything else in the node UI)
        //       -> one option: cache everything and update using callbacks (much like
        //          node positions currently)
        //           - could use dom::Element::onMove to detect when pegs move within
        //             their container, but not when the parent container moves :(
        //       -> another option: recompute all the time
        //           - during render()? Meh, will lag
        //           - when else?
        //       -> another option: add something like React's ref
        //           - can get relative position of pegs, but when to update? same problem as above
        //       -> another option: introduce a new system for observer dom::Element things
        //           - things that could be desired:
        //             - element position (relative to another)
        //             - element size
        //             - ?
        //             - position and size are more controlled by layout than anything else,
        //               so probably no other things need watching. Ergo, a small additional
        //               interface for tracking size and relative position is probably fine
        // 

    }

    Graph* const m_graph;
    
    using NodePosition = std::pair<Node*, Value<vec2>>;
    using PegPosition = std::pair<Node*, Value<vec2>>;
    
    Value<std::vector<NodePosition>> m_nodePositions;
    Value<vec2> m_rootPosition;
    Value<std::vector<PegPosition>> m_inputPegPositions;
    Value<std::vector<PegPosition>> m_outputPegPositions;
};

int main(){
    
    auto graph = Graph{};

    auto& sn = graph.add<StringNode>("Blab blab");
    auto& in = graph.add<IntegerNode>(99);
    auto& bn = graph.add<BooleanNode>(false);
    sn.addInput(&bn);
    bn.addInput(&in);
    in.addInput(&sn);

    bn.addInput(&sn);
    in.addInput(&bn);
    sn.addInput(&in);

    auto comp = AnyComponent{UseFont(&getFont()).with(
        GraphUI{&graph}
    )};

    /*auto comp = AnyComponent{UseFont(&getFont()).with(
        MixedContainerComponent<VerticalListBase, Boxy, Resizable>{TopToBottom, true}
            .sizeForce(vec2{100.0f, 100.0f})
            .backgroundColor(0xB0B0B0FF)
            .containing(
                Expand(AlignLeft{Text("A")}),
                CenterHorizontally{Text("B")},
                AlignRight{Text("C")}
            )
    )};*/

    auto root = Root(FreeContainer{}.containing(std::move(comp)));

    Window& win = Window::create(std::move(root), 600, 400, "Test");

    run();

    return 0;
    
    
    /*
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
    using VectorOfItems = std::vector<std::pair<int, String>>;

    auto someNumber = Value<float>{5.0f};
    auto someBoolean = Value<bool>{true};
    auto someItems = Value<VectorOfItems>{VectorOfItems{{1, "One"}, {2, "Two"}, {3, "Three"}}};
    auto currentItem = Value<std::size_t>{static_cast<std::size_t>(-1)};
    auto toggleState = Value<bool>{false};
    auto theString = Value<String>{"Enter text here..."};
    auto theDouble = Value<double>{0.001};
    auto theInt = Value<int>{99};

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
    */
}
