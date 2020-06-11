#pragma once

#include <GUI/DOM/Element.hpp>
#include <GUI/Util/Key.hpp>

namespace ui::dom {
    
    class Draggable;

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
        
        virtual void onMouseOver();
        virtual void onMouseOut();

        virtual bool onHover(Draggable*);
        virtual bool onDrop(Draggable*);

        virtual bool onKeyDown(Key);
        virtual void onKeyUp(Key);

        bool keyDown(Key) const;

        virtual bool onScroll(vec2);

        virtual void onGainFocus();
        virtual void onLoseFocus();

        bool hasFocus() const;
        void grabFocus();

    protected:
        void transferEventResposeTo(Control* other);

    private:
        Control* toControl() noexcept override final;
    };

} // namespace ui::dom
