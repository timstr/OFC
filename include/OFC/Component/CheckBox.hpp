#pragma once

#include <OFC/Component/Component.hpp>
#include <OFC/Component/PureComponent.hpp>

namespace ofc::ui {

    class CheckBox : public PureComponent {
    public:
        CheckBox(Valuelike<bool> value);

        CheckBox& onChange(std::function<void(bool)> f);

    private:
        Valuelike<bool> m_value;
        std::function<void(bool)> m_onChange;

        AnyComponent render() const override final;
    };

} // namespace ofc::ui
