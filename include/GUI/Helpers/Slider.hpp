#pragma once

#include <GUI/Control.hpp>
#include <GUI/Draggable.hpp>
#include <GUI/FreeContainer.hpp>
#include <GUI/BoxElement.hpp>

namespace ui {

    // TODO: left/right/home/end to adjust
    // TODO: shift+left/right for fine adjustment

    template<typename Number>
    class Slider : public Control, public FreeContainer, public BoxElement {
    public:
        Slider(Number default_val, Number min, Number max, const sf::Font& font, std::function<void(Number)> onChange);

        void setValue(Number);
        Number value();

        // TODO: colours

    private:

        class Dragger;

        void render(sf::RenderWindow&) override;

        void updateFromDragger();

        void updateFromValue();

        bool onLeftClick(int) override;

        Number m_value;
        Number m_minimum;
        Number m_maximum;
        std::function<void(Number)> m_onChange;
        Text& m_label;
        Dragger& m_dragger;
    };




    template<typename Number>
    class Slider<Number>::Dragger : public Draggable, public Control, public BoxElement {
    public:
        Dragger(Slider& parent)
            : m_parent(parent) {
            
        }

        bool onLeftClick(int) override {
            startDrag();
            return true;
        }

        void onLeftRelease() override {
            stopDrag();
        }

        void onDrag() override {
            const auto max = m_parent.width() - width();
            const auto x = std::clamp(left(), 0.0f, max);
            setLeft(x);
            setTop(0.0f);
            m_parent.updateFromDragger();
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
    inline void Slider<Number>::render(sf::RenderWindow& rw){
        BoxElement::render(rw);
        FreeContainer::render(rw);
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
        auto mp = localMousePos();
        assert(mp);
        const auto x = (mp->x - m_dragger.width() * 0.5f);
        m_dragger.setLeft(x);
        m_dragger.startDrag();
        transferEventResposeTo(&m_dragger);
        return true;
    }

} // namespace ui
