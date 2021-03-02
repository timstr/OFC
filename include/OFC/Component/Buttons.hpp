#pragma once

#include <OFC/Component/PureComponent.hpp>

namespace ofc::ui {

    class Button : public PureComponent {
    public:
        Button(Value<String> s);

        Button&& onClick(std::function<void()> f);

    private:
        Value<String> m_caption;
        std::function<void()> m_onClick;

        AnyComponent render() const override final;
    };

    class Toggle : public PureComponent {
    public:
        Toggle(Value<String> falseLabel, Value<String> trueLabel, Value<bool> value);

        Toggle&& onChange(std::function<void(bool)>);

    private:
        Value<String> m_falseLabel;
        Value<String> m_trueLabel;
        Value<bool> m_value;
        std::function<void(bool)> m_onChange;

        AnyComponent render() const override final;
    };

} // namespace ofc::ui
