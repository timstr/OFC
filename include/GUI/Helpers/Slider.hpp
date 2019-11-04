#pragma once

#include <GUI/Control.hpp>
#include <GUI/Draggable.hpp>
#include <GUI/FreeContainer.hpp>
#include <GUI/BoxElement.hpp>

namespace ui {

    // TODO: safety checks in case max==min

    template<typename Number>
    class Slider : public Control, public ui::Boxed<ui::FreeContainer> {
    public:
        Slider(Number default_val, Number min, Number max, const sf::Font& font, std::function<void(Number)> onChange);

        void setValue(Number);
        Number value() const;

        void setMinimum(Number);
        Number minimum() const;

        void setMaximum(Number);
        Number maximum() const;

    private:

        class Dragger;

        void updateFromDragger();

        void updateFromValue();

        bool onLeftClick(int) override;

        bool onKeyDown(Key) override;

        void onResize() override;

        Number m_value;
        Number m_minimum;
        Number m_maximum;
        std::optional<float> m_fineDragStartPos;
        std::function<void(Number)> m_onChange;
        Text& m_label;
        Dragger& m_dragger;
    };


    // TODO: move the following definitions to a .tpp file

    template<typename Number>
    class Slider<Number>::Dragger : public Draggable, public Control, public BoxElement {
    public:
        Dragger(Slider& parent)
            : m_parent(parent) {
            
        }

        bool onLeftClick(int) override {
            if (keyDown(Key::LShift) || keyDown(Key::RShift)){
                m_parent.m_fineDragStartPos = left();
            }
            startDrag();
            return true;
        }

        void onLeftRelease() override {
            stopDrag();
            m_parent.m_fineDragStartPos.reset();
        }

        void onDrag() override {
            const auto max = m_parent.width() - width();
            const auto l = m_parent.m_fineDragStartPos ?
                (left() * 0.1f + 0.9f * *m_parent.m_fineDragStartPos) :
                left();
            const auto x = 
                std::clamp(l, 0.0f, max);
            setLeft(x);
            setTop(0.0f);
            const auto t = static_cast<double>(x / max);
            const auto v = static_cast<double>(m_parent.m_minimum)
                + t * static_cast<double>(m_parent.m_maximum - m_parent.m_minimum);
            assert(m_parent.m_onChange);
            if constexpr (std::is_integral_v<Number>){
                m_parent.m_value = static_cast<Number>(std::round(v));
                m_parent.m_onChange(m_parent.m_value);
            } else {
                m_parent.m_value = static_cast<Number>(v);
                m_parent.m_onChange(m_parent.m_value);
            }
        }

    private:
        Slider& m_parent;
    };

    

    template<typename Number>
    inline Slider<Number>::Slider(Number defaultValue, Number min, Number max, const sf::Font& font, std::function<void(Number)> onChange)
        : m_value(defaultValue)
        , m_minimum(min)
        , m_maximum(max)
        , m_onChange(std::move(onChange))
        , m_label(add<Text>(FreeContainer::InsideLeft, FreeContainer::Center, "", font))
        , m_dragger(add<Dragger>(*this)) {

        static_assert(std::is_arithmetic_v<Number>, "Slider must use a numeric type as a template parameter");

        setBackgroundColor(0xDDDDDDFF);
        setSize({100.0f, 20.0f}, true);
        setBorderRadius(10.0f);
        setBorderThickness(2.0f);
        setBorderColor(0x888888FF);

        m_dragger.setBackgroundColor(0xFFFFFF80);
        m_dragger.setSize({20.0f, 20.0f});
        m_dragger.setBorderRadius(10.0f);
        m_dragger.setBorderThickness(2.0f);
        m_dragger.setBorderColor(0x888888FF);

        updateFromValue();
    }

    template<typename Number>
    inline void Slider<Number>::setValue(Number v){
        m_value = std::clamp(v, m_minimum, m_maximum);
        updateFromValue();
    }

    template<typename Number>
    inline Number Slider<Number>::value() const {
        return m_value;
    }

    template<typename Number>
    inline void Slider<Number>::setMinimum(Number v){
        m_minimum = v;
        m_maximum = std::max(m_maximum, m_minimum);
        auto vc = std::clamp(m_value, m_minimum, m_maximum);
        if (vc != m_value){
            m_value = vc;
            updateFromValue();
            assert(m_onChange);
            m_onChange(m_value);
        }
    }

    template<typename Number>
    inline Number Slider<Number>::minimum() const {
        return m_minimum;
    }

    template<typename Number>
    inline void Slider<Number>::setMaximum(Number v){
        m_maximum = v;
        m_minimum = std::min(m_minimum, m_maximum);
        auto vc = std::clamp(m_value, m_minimum, m_maximum);
        if (vc != m_value){
            m_value = vc;
            updateFromValue();
            assert(m_onChange);
            m_onChange(m_value);
        }
    }

    template<typename Number>
    inline Number Slider<Number>::maximum() const {
        return m_maximum;
    }

    template<typename Number>
    inline void Slider<Number>::updateFromDragger(){
        const auto x = m_dragger.left();
        const auto max = width() - m_dragger.width();
        const auto t = static_cast<double>(x) / static_cast<double>(max);
        const auto v = static_cast<double>(m_minimum) + t * static_cast<double>(m_maximum - m_minimum);

        assert(m_onChange);
        if constexpr (std::is_integral_v<Number>){
            const auto v1 = static_cast<Number>(std::round(v));
            if (m_value != v1){
                m_value = v1;
                m_onChange(m_value);
            }
            updateFromValue();
        } else {
            const auto v1 = static_cast<Number>(v);
            if (m_value != v1){
                m_value = v1;
                m_onChange(m_value);
                m_label.setText(std::to_string(m_value));
            }
        }
    }

    template<typename Number>
    inline void Slider<Number>::updateFromValue(){
        const auto v = static_cast<float>(m_value);
        const auto t = (v - static_cast<float>(m_minimum)) / static_cast<float>(m_maximum - m_minimum);
        assert(t >= 0.0 && t <= 1.0);
        const auto max = width() - m_dragger.width();
        m_dragger.setLeft(t * max);
        m_label.setText(std::to_string(m_value));
    }

    template<typename Number>
    bool Slider<Number>::onLeftClick(int){
        if (keyDown(Key::LShift) || keyDown(Key::RShift)){
            m_fineDragStartPos = m_dragger.left();
        } else {
            auto mp = localMousePos();
            const auto x = (mp.x - m_dragger.width() * 0.5f);
            m_dragger.setLeft(x);
        }
        m_dragger.startDrag();
        transferEventResposeTo(&m_dragger);
        return true;
    }

    template<typename Number>
    inline bool Slider<Number>::onKeyDown(Key key){
        const auto shift = keyDown(Key::LShift) || keyDown(Key::RShift);
        const auto ctrl = keyDown(Key::LControl) || keyDown(Key::RControl);

        const auto adjust = [&](double sign){
            // normal speed: power of ten that is closest to moving one pixel,
            // or simply 1 if Number is integral and that speed would be less than 1
            // coarse speed: 10x normal
            // fine speed: 0.1x normal (minimum of 1 if integral)
            const auto multiplier = ctrl ? 10.0 : (shift ? 0.1 : 1.0);
            const auto spacePerPixel =
                static_cast<double>(m_maximum - m_minimum)
                / static_cast<double>(width() - m_dragger.width());
            const auto mag = std::log10(spacePerPixel);
            const auto baseStep = std::pow(10.0, std::round(mag)) * multiplier;
            
            Number step;
            if (std::is_integral_v<Number> && baseStep < 1.0){
                step = static_cast<Number>(sign);
            } else {
                step = static_cast<Number>(baseStep * sign);
            }
            
            const auto v = std::clamp(
                m_value + step,
                m_minimum,
                m_maximum
            );
            if (m_value != v){
                m_value = v;
                assert(m_onChange);
                m_onChange(m_value);
                updateFromValue();
            }
        };

        if (key == Key::Home){
            m_value = m_minimum;
            assert(m_onChange);
            m_onChange(m_value);
            updateFromValue();
            return true;
        } else if (key == Key::End){
            m_value = m_maximum;
            assert(m_onChange);
            m_onChange(m_value);
            updateFromValue();
            return true;
        } else if (key == Key::Left){
            adjust(-1.0);
            return true;
        } else if (key == Key::Right){
            adjust(1.0);
            return true;
        }

        return false;
    }
    
    template<typename Number>
    inline void Slider<Number>::onResize(){
        BoxElement::onResize();
        assert(width() > height());
        setBorderRadius(height() / 2.0f);
        m_dragger.setSize({height(), height()}, true);
        m_dragger.setBorderRadius(height() / 2.0f);
        updateFromValue();
    }

} // namespace ui
