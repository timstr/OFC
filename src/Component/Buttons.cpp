#include <OFC/Component/Buttons.hpp>

#include <OFC/Component/MixedComponent.hpp>
#include <OFC/Component/Text.hpp>

namespace ofc::ui {

    Button::Button(Value<String> s)
        : m_caption(std::move(s)) {

    }

    Button&& Button::onClick(std::function<void()> f) {
        m_onClick = std::move(f);
        return std::move(*this);
    }

    AnyComponent Button::render() const {
        return MixedContainerComponent<FlowContainerBase, Boxy, Clickable, Resizable>{}
            .minSize(vec2{50.0f, 20.0f})
            .backgroundColor(0xddddddff)
            .borderColor(0x888888ff)
            .borderThickness(2.0f)
            .borderRadius(5.0f)
            .onLeftClick([this](int){
                if (m_onClick){
                    m_onClick();
                }
                return true;
            })
            .containing(Text(m_caption));
    }
    
    Toggle::Toggle(Value<String> falseLabel, Value<String> trueLabel, Value<bool> value)
        : m_falseLabel(std::move(falseLabel))
        , m_trueLabel(std::move(trueLabel))
        , m_value(std::move(value)) {

    }

    Toggle&& Toggle::onChange(std::function<void(bool)> f) {
        m_onChange = std::move(f);
        return std::move(*this);
    }

    AnyComponent Toggle::render() const {
        auto getCurrentLabel = [](const String& fl, const String& tl, bool v){
            return v ? tl : fl;
        };

        auto currentLabel = combine(m_falseLabel, m_trueLabel, m_value)
            .map(std::move(getCurrentLabel));

        return MixedContainerComponent<FlowContainerBase, Boxy, Clickable, Resizable>{}
            .minSize(vec2{50.0f, 20.0f})
            .backgroundColor(0xddddddff)
            .borderColor(0x888888ff)
            .borderThickness(2.0f)
            .borderRadius(5.0f)
            .onLeftClick([this](int){
                if (m_onChange){
                    m_onChange(!m_value.getOnce());
                }
                return true;
            })
            .containing(Text(std::move(currentLabel)));
    }

} // namespace ofc::ui
