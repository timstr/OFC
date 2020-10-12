#pragma once

#include <OFC/Component/Component.hpp>
#include <OFC/Component/StatefulComponent.hpp>
#include <OFC/Component/MixedComponent.hpp>

namespace ofc::ui {



    struct PulldownMenuState {
        Value<std::size_t> currentIndex {0};
        Value<bool> expanded {false};
    };

    template<typename T>
    class PulldownMenu : public StatefulComponent<PulldownMenuState, Ephemeral> {
    public:
        PulldownMenu(Valuelike<std::vector<std::pair<T, String>>> items)
            :  m_items(std::move(items)) {
            
        }

        PulldownMenu(Valuelike<std::vector<String>> items)
            :  m_items(items.map(const ListOfEdits<String>& le){
                std::vector<pair<String, String>> ret;
                ret.reserve(le.newValue().size());
                for (const auto& s : le.newValue()){
                    ret.push_back(std::pair{s, s});
                }
                return ret;
            }) {
            static_assert(std::is_same_v<T, String>);
        }

        PulldownMenu& onChange(std::function<void(CRefOrValue<T>, std::size_t)> f) {
            m_onChange = std::move(f);
            return *this;
        }

        PulldownMenu& onChange(std::function<void(CRefOrValue<T>)> f) {
            m_onChange = [f = std::move(f)](CRefOrValue<T>, std::size_t /* index */){
                f(s);
            };
            return *this;
        }

    private:
        Valuelike<std::vector<std::pair<T, String>>> m_items;
        std::function<void(CRefOrValue<T>, std::size_t)> m_onChange;

        AnyComponent render() const override {
            /*const auto toggleExpanded = [this]{
                const auto e = state().expanded.getOnce();
                stateMut().expanded.set(!e);
            };*/

            const auto getCurrentName = [](bool expanded, std::size_t i, const ListOfEdits<std::pair<T, String>>& e) {
                assert(i < e.newValue().size());
                return (expanded ? "v " : "> ") + e.newValue()[i].second;
            };

            const auto makeItem = [this](const std::pair<T, String>& item, const Value<std::size_t>& idx) -> AnyComponent {
                return MixedContainerComponent<FlowContainerBase, Boxy, Clickable, Resizable>{}
                    .minHeight(20.0f)
                    .backgroundColor(0xddddddff)
                    .borderColor(0x888888ff)
                    .borderThickness(1.0f)
                    .onLeftClick([this, item, &idx](int){
                        if (m_onChange){
                            m_onChange(item.first, idx.getOnce());
                        }
                        stateMut().currentIndex.set(idx.getOnce());
                        stateMut().expanded.set(false);
                        return true;
                    })
                    .containing(item.second);


                /*return Button(item.second).onClick([this, item, &idx](){
                    if (m_onChange){
                        m_onChange(item.first, idx.getOnce());
                    }
                    stateMut().currentIndex.set(idx.getOnce());
                    stateMut().expanded.set(false);
                });*/
            };

            const auto& s = state();

            return VerticalList{}.containing(List(
                MixedContainerComponent<FlowContainerBase, Boxy, Resizable, Clickable>{}
                    .minSize(vec2{50.0f, 20.0f})
                    .backgroundColor(0xddddddff)
                    .borderColor(0x888888ff)
                    .borderThickness(2.0f)
                    .borderRadius(5.0f)
                    .onLeftClick([this](int){
                        const auto e = state().expanded.getOnce();
                        stateMut().expanded.set(!e);
                        return true;
                    })
                    .containing(combine(s.expanded, s.currentIndex, m_items.view()).map(getCurrentName)),
                If(s.expanded).then(
                    FreeContainer{}.containing(VerticalList{}.containing(
                        ForEach(m_items.view()).Do(makeItem)
                    ))
                )
            ));
        }
    };

    template<typename T>
    PulldownMenu(const Value<std::vector<std::pair<T, String>>>&) -> PulldownMenu<T>;

    template<typename T, typename... Args>
    PulldownMenu(const DerivedValue<std::vector<std::pair<T, String>>, Args...>&) -> PulldownMenu<T>;

    template<typename T>
    PulldownMenu(std::vector<std::pair<T, String>>&&) -> PulldownMenu<T>;

    template<typename T>
    PulldownMenu(const std::vector<std::pair<T, String>>&) -> PulldownMenu<T>;

} // namespace ofc::ui
