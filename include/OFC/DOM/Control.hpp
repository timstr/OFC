#pragma once

#include <OFC/DOM/Element.hpp>
#include <OFC/Util/Key.hpp>

namespace ofc::ui::dom {
    
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
        
        virtual void onMouseEnter(Draggable*);
        virtual void onMouseLeave(Draggable*);

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

} // namespace ofc::ui::dom
