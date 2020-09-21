#pragma once

#include <OFC/Component/PureComponent.hpp>
#include <OFC/Component/MixedComponent.hpp>
#include <OFC/Component/Text.hpp>

namespace ofc::ui {

    template<typename NumberType>
    class Slider : public PureComponent {
    public:
        Slider(Valuelike<NumberType> minimum, Valuelike<NumberType> maximum, Valuelike<NumberType> value)
            : m_minimum(std::move(minimum))
            , m_maximum(std::move(maximum))
            , m_value(std::move(value)) {
        
        }

        Slider& onChange(std::function<void(NumberType)> f) {
            m_onChange = std::move(f);
            return *this;
        }

    private:
        Valuelike<NumberType> m_minimum;
        Valuelike<NumberType> m_maximum;
        Valuelike<NumberType> m_value;
        std::function<void(NumberType)> m_onChange;

        AnyComponent render() const override {
            auto valueAsString = m_value.map([](NumberType x) -> String {
                return std::to_string(x);
            });

            auto leftPosition = combine(m_minimum.view(), m_maximum.view(), m_value.view())
                .map([](NumberType min, NumberType max, NumberType v){
                    assert(max >= min);
                    if (max == min){
                        return 80.0f / 2.0f; // TODO: (width - height)
                    }
                    // TODO: (width - height)
                    return 80.0f * std::clamp(static_cast<float>(v - min) / static_cast<float>(max - min), 0.0f, 1.0f);
                });

            auto onDrag = [this](vec2 v){
                const auto min = m_minimum.getValueOnce();
                const auto max = m_maximum.getValueOnce();
                // TODO: (width - height)
                const auto w = 80.0f;
                const auto x = std::clamp(v.x / w, 0.0f, 1.0f);
                const auto val = static_cast<NumberType>(
                    x * static_cast<float>(max - min) + static_cast<float>(min)
                );
                m_onChange(val);
                return vec2{std::clamp(v.x, 0.0f, w), 0.0f};
            };

            // auto handleKeyDown = [this]()

            return MixedContainerComponent<FreeContainerBase, Boxy, Resizable, KeyPressable>{}
                .sizeForce(vec2{100.0f, 20.0f})
                .backgroundColor(0xddddddff)
                .borderColor(0x888888ff)
                .borderThickness(2.0f)
                .borderRadius(10.0f)
                .containing(
                    Text(std::move(valueAsString)),
                    MixedComponent<Boxy, Resizable, Positionable, Clickable, Draggable>{}
                        .sizeForce(vec2{20.0f, 20.0f})
                        .backgroundColor(0xffffff80)
                        .borderColor(0x888888ff)
                        .borderThickness(2.0f)
                        .borderRadius(10.0f)
                        .top(0.0f)
                        .left(std::move(leftPosition))
                        .onLeftClick([](int, ModifierKeys, auto action){
                            action.startDrag();
                            return true;
                        })
                        .onLeftRelease([](auto action){
                            action.stopDrag();
                            return true;
                        })
                        .onDrag(onDrag)
                );
        }
    };

    // TODO: toggle button

} // namespace ofc::ui
