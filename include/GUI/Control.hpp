#pragma once

#include <GUI/Element.hpp>
#include <GUI/Draggable.hpp>

namespace ui {
    
    using Key = sf::Keyboard::Key;

    class Control : virtual public Element {
    public:

        virtual bool onLeftClick(int);
        virtual bool onMiddleClick(int);
        virtual bool onRightClick(int);
        
        virtual void onLeftRelease();
        virtual void onMiddleRelease();
        virtual void onRightRelease();

        bool leftMouseDown() const;
        bool middleMouseDown() const;
        bool rightMouseDown() const;
        
        virtual void onMouseOver(Draggable*);
        virtual void onMouseOut();

        virtual bool onDrop(Draggable*);

        virtual bool onKeyDown(Key);
        virtual void onKeyUp(Key);

        bool keyDown(Key) const;

        virtual bool onScroll(vec2);

        virtual void onGainFocus();
        virtual void onLoseFocus();

        bool hasFocus() const;
        void grabFocus();
    };

} // namespace ui