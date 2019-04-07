#pragma once

#include <SFML/Graphics.hpp>
#include <GUI/Transition.hpp>

namespace ui {
    
    using vec2 = sf::Vector2f;

    enum class LayoutStyle : uint8_t;

    class Container;
    class Control;
    class Draggable;
    class Text;
    class TextEntry;

    class Element {
    public:
        Element();
        virtual ~Element();

        float left() const;
        float top() const;
        vec2 pos() const;

        void setLeft(float);
        void setTop(float);
        void setPos(vec2);

        virtual void onMove();

        float width() const;
        float height() const;
        vec2 size() const;

        void setWidth(float);
        void setHeight(float);
        void setSize(vec2);

        virtual void onResize();

        float margin() const;

        void setMargin(float);

        virtual bool hit(vec2 p) const;

        virtual void render(sf::RenderWindow&) = 0;

    private:
        vec2 m_position;
        vec2 m_size;
        float m_margin;
        LayoutStyle m_layoutstyle;

        Container* m_parent;

        class Transition {
            public:
            Transition(double duration, std::function<void(double)> fn, std::function<void()> on_complete = {});
            ~Transition();

        private:
            double m_duration;
            std::function<void(double)> m_fn;
            std::function<void()> m_on_complete;


            friend class Element;
        };

        std::vector<Transition> m_transitions;

        virtual Container* toContainer();
        virtual Control* toControl();
        virtual Draggable* toDraggable();
        virtual Text* toText();
        virtual TextEntry* toTextEntry();
    };

} // namespace ui