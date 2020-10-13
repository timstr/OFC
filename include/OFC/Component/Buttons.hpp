#pragma once

#include <OFC/Component/PureComponent.hpp>

namespace ofc::ui {

    class Button : public PureComponent {
    public:
        Button(Valuelike<String> s);

        Button& onClick(std::function<void()> f);

    private:
        Valuelike<String> m_caption;
        std::function<void()> m_onClick;

        AnyComponent render() const override final;
    };

    class Toggle : public PureComponent {
    public:
        Toggle(Valuelike<String> falseLabel, Valuelike<String> trueLabel, Valuelike<bool> value);

        Toggle& onChange(std::function<void(bool)>);

    private:
        Valuelike<String> m_falseLabel;
        Valuelike<String> m_trueLabel;
        Valuelike<bool> m_value;
        std::function<void(bool)> m_onChange;

        AnyComponent render() const override final;
    };

} // namespace ofc::ui
