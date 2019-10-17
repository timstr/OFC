#pragma once

#include <SFML/Graphics.hpp>

#include <cassert>
#include <functional>
#include <optional>

namespace ui {
    
    using vec2 = sf::Vector2f;

    using String = sf::String;
    
    class Container;
    class Control;
    class Draggable;
    class Text;
    class TextEntry;
    class Window;

    class Element {
    public:
        Element();
        virtual ~Element();

        Element(const Element&) = delete;
        Element(Element&&) = delete;
        Element& operator=(const Element&) = delete;
        Element& operator=(Element&&) = delete;

        // the element's position relative to its container
        float left();
        float top();
        vec2 pos();
        void setLeft(float);
        void setTop(float);
        void setPos(vec2);

        // the element's position relative to the window
        vec2 rootPos();

        // The position of the mouse relative to the element
        std::optional<vec2> localMousePos();

        // called whenever the element's position is changed
        virtual void onMove();

        // get the element's size
        float width();
        float height();
        vec2 size();

        // set the element's size
        void setWidth(float, bool force = false);
        void setMinWidth(float);
        void setMaxWidth(float);
        void setHeight(float, bool force = false);
        void setMinHeight(float);
        void setMaxHeight(float);
        void setSize(vec2, bool force = false);
        void setMinSize(vec2);
        void setMaxSize(vec2);

        // called whenever the element's size is changed
        virtual void onResize();

        // returns true if the point p collides with the
        // the element (in local coordinates)
        virtual bool hit(vec2 p) const;

        // find an element that is hit at the given position
        // (in local coordinates)
        virtual Element* findElementAt(vec2 p);

        // draws the window
        virtual void render(sf::RenderWindow&) = 0;

        // start a transition
        //  duration    - total time the transition will take, in seconds
        //  fn          - called each tick of the animation, where the argument
        //                is the progress, and varies from 0 to 1
        //  on_complete - (optional) called when the animation ends
        void startTransition(double duration, std::function<void(double)> fn, std::function<void()> on_complete = {});

        void clearTransitions();

        // removes the element from its parent and returns a unique_ptr
        // containing the element.
        // Throws an exception if the element has no parent
        std::unique_ptr<Element> orphan();

        // Cause the element to be rendered in front of its siblings
        void bringToFront();

        // Called whenever the element is removed from the UI, either
        // directly (from its parent container) or indirectly.
        virtual void onRemove();

        // Removes the element from its parent and destroys it.
        // Throws an exception if the element has no parent.
        // DO NOT use the element after calling close.
        // Dereferencing the element will cause Undefined Behavior.
        void close();

        // get the parent container
        // returns null if the element does not belong to a container
        Container* getParentContainer();
        const Container* getParentContainer() const;

        // Get the parent window
        // returns null if the element does not belong to a window
        Window* getParentWindow() const;

        // find the nearest parent container that is also a control
        // returns null if none is found
        Control* getParentControl();
        const Control* getParentControl() const;

        /*
        If an element's size is changed internally (via setSize() or if it is a container
        and just changed size due to modified contents):
            - Propagate the change to the parent container (this may trigger some
              additional updates)
                -> maybe a method like Container::onContentsChanged() can be used here
                   which will recompute the layout of its children recursively
                -> On second though, this sounds redundent. A simple virtual
                   Element::update() probably suffices

        If more space becomes available to an element with horizontal or vertical fill
        enabled:
            - It is resized to take up that space, and if it is a container, its
              layout is recomputed, using update()

        If an element is dirty (needs to be updated), should it:
            - be put into a global rerender queue for efficiency?
                -> If this is done, some care should be taken to prevent
                   updating the same element twice (i.e. if both a container
                   and its child are queued and updating one also causes the
                   other to be made clean, it can be removed from the queue
            - be made clean by a recursive update() function called on
              the entire UI tree?
                -> This will waste a lot of CPU time/power


        If an element is updated and its size changes, should it
            - Call on its parent container to update() also?
            - Mark its parent container (depending on its layout style) as dirty
              and queue it for updating?
                -> This could (should?) be done by the queuing code, not in each
                   different update() method
        */

        // Marks the element as dirty
        void requireUpdate();

    private:
        // Make the element's contents up to date.
        // This used mainly by Containers to position and resize their children
        virtual vec2 update();

        // Causes the element to immediately be updated if it or its parent have
        // been marked dirty.
        // This method is called when the element's size or position is queried.
        void forceUpdate();

        void requireDeepUpdate();

    private:
        mutable vec2 m_position;
        mutable vec2 m_size;
        vec2 m_minsize;
        vec2 m_maxsize;

        bool m_needs_update;
        bool m_isUpdating;

        Container* m_parent;

        virtual Container* toContainer();
        const Container* toContainer() const;
        virtual Control* toControl();
        const Control* toControl() const;
        virtual Draggable* toDraggable();
        const Draggable* toDraggable() const;
        virtual Text* toText();
        const Text* toText() const;
        virtual TextEntry* toTextEntry();
        const TextEntry* toTextEntry() const;

        virtual Window* getWindow() const;

        friend class Container;
        friend class Control;
        friend class Window;
    };

    template<typename T>
    std::unique_ptr<T> makeOrphan(T& element){
        static_assert(std::is_base_of_v<Element, T>, "T must derive from Element");
        auto ptr = dynamic_cast<T*>(element.orphan().release());
        assert(ptr);
        return std::unique_ptr<T>{ptr};
    }

} // namespace ui