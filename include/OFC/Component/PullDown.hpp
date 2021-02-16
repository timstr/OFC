#pragma once

#include <OFC/Component/Component.hpp>
#include <OFC/Component/StatefulComponent.hpp>
#include <OFC/Component/MixedComponent.hpp>
#include <OFC/Component/Text.hpp>

namespace ofc::ui {

    struct PulldownMenuState {
        Value<bool> expanded {false};
    };

    template<typename T>
    class PulldownMenu : public StatefulComponent<PulldownMenuState, Ephemeral> {
    public:
        PulldownMenu(Value<std::vector<std::pair<T, String>>> items, Value<std::size_t> currentIndex)
            :  m_items(std::move(items))
            ,  m_currentIndex(std::move(currentIndex)) {
            
        }

        PulldownMenu(Value<std::vector<String>> items)
            :  m_items(items.vectorMap([](const String& s){
                return std::pair{s, s};
            })) {
            static_assert(std::is_same_v<T, String>);
        }

        PulldownMenu& onChange(std::function<void(CRefOrValue<T>, std::size_t)> f) {
            m_onChange = std::move(f);
            return *this;
        }

    private:
        Value<std::vector<std::pair<T, String>>> m_items;
        Value<std::size_t> m_currentIndex;
        std::function<void(CRefOrValue<T>, std::size_t)> m_onChange;

        AnyComponent render() const override {
            const auto getCurrentName = [](bool expanded, std::size_t i, const ListOfEdits<std::pair<T, String>>& e) {
                const auto& v = e.newValue();
                auto prefix = String(expanded ? "v " : "> ");
                if (i < v.size()){
                    return prefix + v[i].second;
                }
                return prefix + "???";
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
                        stateMut().expanded.set(false);
                        return true;
                    })
                    .containing(Text(item.second));
            };

            const auto& s = state();

            return VerticalList{}.containing(List(
                MixedContainerComponent<FlowContainerBase, Boxy, Resizable, Clickable, Focusable>{}
                    .minSize(vec2{50.0f, 20.0f})
                    .backgroundColor(0xddddddff)
                    .borderColor(0x888888ff)
                    .borderThickness(2.0f)
                    .borderRadius(5.0f)
                    .onLeftClick([this](int){
                        auto& e = stateMut().expanded;
                        e.set(!e.getOnce());
                        return true;
                    })
                    .onLoseFocus([this]{
                        stateMut().expanded.set(false);
                    })
                    .containing(Text(combine(s.expanded, m_currentIndex, m_items).map(getCurrentName))),
                If(s.expanded).then(
                    FreeContainer{}.containing(VerticalList{}.containing(
                        ForEach(m_items).Do(makeItem)
                    ))
                )
            ));
        }
    };

    template<typename T>
    PulldownMenu(Value<std::vector<std::pair<T, String>>>, Value<std::size_t>) -> PulldownMenu<T>;

    template<typename T>
    PulldownMenu(std::vector<std::pair<T, String>>&&, Value<std::size_t>) -> PulldownMenu<T>;

    template<typename T>
    PulldownMenu(const std::vector<std::pair<T, String>>&, Value<std::size_t>) -> PulldownMenu<T>;

} // namespace ofc::ui
