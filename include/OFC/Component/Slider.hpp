#pragma once

#include <OFC/Component/StatefulComponent.hpp>
#include <OFC/Component/MixedComponent.hpp>
#include <OFC/Component/Text.hpp>

#include <optional>

namespace ofc::ui {

    template<typename NumberType>
    struct SliderState {
        std::optional<float> startPosition;
    };

    template<typename NumberType>
    class Slider : public StatefulComponent<SliderState<NumberType>, Ephemeral> {
    public:
        Slider(Valuelike<NumberType> minimum, Valuelike<NumberType> maximum, Valuelike<NumberType> value)
            : m_minimum(std::move(minimum))
            , m_maximum(std::move(maximum))
            , m_value(std::move(value)) {
        
            static_assert(std::is_arithmetic_v<NumberType>);
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
                const auto& s = this->state();
                const auto x = s.startPosition.has_value() ?
                    (0.9f * (*s.startPosition) + 0.1f * v.x) :
                    v.x;
                const auto t = std::clamp(x / w, 0.0f, 1.0f);
                const auto val = static_cast<NumberType>(
                    t * static_cast<float>(max - min) + static_cast<float>(min)
                );
                m_onChange(val);
                return vec2{std::clamp(x, 0.0f, w), 0.0f};
            };

            auto handleKeyDown = [this](Key k, ModifierKeys mod){
                if (k == Key::Home){
                    m_onChange(m_minimum.getValueOnce());
                    return true;
                }
                if (k == Key::End){
                    m_onChange(m_maximum.getValueOnce());
                    return true;
                }

                if (k == Key::Left || k == Key::Right){
                    // normal speed: power of ten that is closest to moving one pixel,
                    // or simply 1 if Number is integral and that speed would be less than 1
                    // coarse speed: 10x normal
                    // fine speed: 0.1x normal (minimum of 1 if integral)
                    const auto multiplier = mod.ctrl() ? 10.0 : (mod.shift() ? 0.1 : 1.0);
                
                        // TODO: (width - height)
                    const auto spacePerPixel =
                        static_cast<float>(m_maximum.getValueOnce() - m_minimum.getValueOnce())
                        / 80.0f;
                    const auto mag = std::log10(spacePerPixel);
                    const auto baseStep = std::pow(10.0f, std::round(mag)) * multiplier;
            
                    NumberType step;
                    if constexpr (std::is_integral_v<NumberType> && baseStep < 1.0f){
                        step = static_cast<NumberType>(1);
                    } else {
                        step = static_cast<NumberType>(baseStep);
                    }


                    auto v = m_value.getValueOnce();
                    if (k == Key::Left){
                        v -= step;
                    } else if (k == Key::Right){
                        v += step;
                    }
                    v = std::clamp(v, m_minimum.getValueOnce(), m_maximum.getValueOnce());
                    m_onChange(v);
                    return true;
                }
                return false;
            };

            return MixedContainerComponent<FreeContainerBase, Boxy, Resizable, KeyPressable>{}
                .sizeForce(vec2{100.0f, 20.0f})
                .backgroundColor(0xddddddff)
                .borderColor(0x888888ff)
                .borderThickness(2.0f)
                .borderRadius(10.0f)
                .containing(
                    Text(std::move(valueAsString)),
                    MixedComponent<Boxy, Resizable, Positionable, Clickable, KeyPressable, Draggable>{}
                        .sizeForce(vec2{20.0f, 20.0f})
                        .backgroundColor(0xffffff80)
                        .borderColor(0x888888ff)
                        .borderThickness(2.0f)
                        .borderRadius(10.0f)
                        .top(0.0f)
                        .left(std::move(leftPosition))
                        .onKeyDown(handleKeyDown)
                        .onLeftClick([this](int, ModifierKeys mod, auto action){
                            if (mod.shift()){
                                auto lPos = action.element().left();
                                stateMut().startPosition = lPos;
                            }
                            action.startDrag();
                            return true;
                        })
                        .onLeftRelease([this](auto action){
                            stateMut().startPosition.reset();
                            action.stopDrag();
                            return true;
                        })
                        .onDrag(onDrag)
                );
        }
    };

    // TODO: toggle button

} // namespace ofc::ui
