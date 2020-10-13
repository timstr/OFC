#include <OFC/Component/CheckBox.hpp>

#include <OFC/Component/If.hpp>
#include <OFC/Component/MixedComponent.hpp>

namespace ofc::ui {
    
    CheckBox::CheckBox(Valuelike<bool> value)
        : m_value(std::move(value)) {

    }

    CheckBox& CheckBox::onChange(std::function<void(bool)> f) {
        m_onChange = std::move(f);
        return *this;
    }

    AnyComponent CheckBox::render() const {
        return MixedContainerComponent<FreeContainerBase, Resizable, Boxy, Clickable>{}
            .sizeForce(vec2{20.0, 20.0})
            .backgroundColor(0xddddddff)
            .borderColor(0x888888ff)
            .borderThickness(2.0f)
            .borderRadius(5.0f)
            .onLeftClick([this](int){
                m_onChange(!m_value.getOnce());
                return true;
            })
            .containing(
                Center(If(m_value.view()).then("X"))
            );
    }

} // namespace ofc::ui
