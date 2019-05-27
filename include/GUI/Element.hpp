#pragma once

#include <SFML/Graphics.hpp>

#include <functional>

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
        float left() const;
        float top() const;
        vec2 pos() const;
        void setLeft(float);
        void setTop(float);
        void setPos(vec2);

        // the element's position relative to the window
        vec2 rootPos() const;

        // The position of the mouse relative to the element
        vec2 localMousePos() const;

        // called whenever the element's position is changed
        virtual void onMove();

        // get the element's size
        float width() const;
        float height() const;
        vec2 size() const;

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

        // whether the element takes up all available space vertically
        // and horizontally (when placed in a suitable container),
        // subject to minimum and maximum size
        void setHorizontalFill(bool);
        void setVerticalFill(bool);
        bool horizontalFill() const;
        bool verticalFill() const;

        // called whenever the element's size is changed
        virtual void onResize();

        // returns true if the point p collides with the
        // the element (in local coordinates)
        virtual bool hit(vec2 p) const;

        // find an element that is hit at the given position
        // (in local coordinates)
        virtual Element* findElementAt(vec2 p) const;

        // draws the window
        virtual void render(sf::RenderWindow&) = 0;

        // start a transition
        //  duration    - total time the transition will take, in seconds
        //  fn          - called each tick of the animation, where the argument
        //                is the progress, and varies from 0 to 1
        //  on_complete - (optional) called when the animation ends
        void startTransition(float duration, std::function<void(float)> fn, std::function<void()> on_complete = {});

        // removes the element from its parent and returns a unique_ptr
        // containing the element.
        // Throws an exception if the element has no parent
        std::unique_ptr<Element> orphan();

        // Removes the element from its parent and destroys it
        // Throws an exception if the element has no parent
        // Do not use the element after calling close.
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

        void require_update();

        // Makes the element up to date, in terms of its
        // size and contents.
        // Called after any size and content related changes
        // are made.
        // max_size is the largest available size, which
        // the element may take up exactly or not, depending
        // on whether horizontal/vertical fill are enabled.
        // Calls updateContents to deal with specialized
        // layout styles.
        void update(vec2 max_size);

    private:
        
        // Should be overridden to make the element's contents
        // up to date, whatever those contents may be
        virtual void updateContents();

    private:
        vec2 m_position;
        vec2 m_size;
        vec2 m_minsize;
        vec2 m_maxsize;
        bool m_fill_x;
        bool m_fill_y;

        bool m_needs_update;

        Container* m_parent;

        struct Transition {
            Transition(float duration, std::function<void(float)>&& fn, std::function<void()>&& on_complete);

            const double duration;
            const std::function<void(float)> fn;
            const std::function<void()> on_complete;
        };

        std::vector<Transition> m_transitions;

        virtual Container* toContainer();
        virtual Control* toControl();
        virtual Draggable* toDraggable();
        virtual Text* toText();
        virtual TextEntry* toTextEntry();

        virtual Window* getWindow() const;

        friend class Container;
        friend class Control;
        friend class Window;
    };

} // namespace ui