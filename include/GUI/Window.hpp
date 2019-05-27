#pragma once

#include <GUI/FreeContainer.hpp>
#include <GUI/Control.hpp>
#include <GUI/TextEntry.hpp>

#include <SFML/Window.hpp>
#include <string>

namespace ui {

    class Window {
    public:
        Window(const Window&) = delete;
        Window(Window&&) = delete;
        Window& operator=(const Window&) = delete;
        Window& operator=(Window&&) = delete;
        ~Window() = default;
        
        static Window& create(unsigned width, unsigned height, const String& title);

        // redraw the UI
        void redraw();

        // the window's inner size
        vec2 getSize() const;
        void setSize(vec2);

        // the on-screen position of the window
        vec2 getPosition() const;
        void setPosition(vec2);

        // the mouse's position relative to the window
        vec2 getMousePosition() const;

        // change the window's title
        void setTitle(const String&);

        // change the window's icon
        void setIcon(const sf::Image&);

        // take a screenshot
        sf::Image screenshot() const;

        // close the window, destroying it in the process
        // DO NOT use the object after calling close()
        void close();

        // the window's OS focus
        bool inFocus() const;
        void requestFocus();

        FreeContainer& root();
        const FreeContainer& root() const;

    private:
        Window(unsigned width, unsigned height, const String& title);

        // process the window's event queue
        void processEvents();

        // emulates the releasing of all held
        // keys and mouse buttons
        void releaseAllButtons();

        // clean up all listeners for a window
        void onRemoveElement(Element*);

        // focus to an element
        // passing null will remove the focus from all elements
        void focusTo(Control*);

        void startDrag(Draggable*, vec2);
        void stopDrag();
        Draggable* currentDraggable();

        void startTyping(TextEntry*);
        void stopTyping();
        TextEntry* currentTextEntry();

    private:

        /********** UI state **********/

        // focus
        Control* m_focus_elem;

        // dragging
        Draggable* m_drag_elem;
        vec2 m_drag_offset;

        // mouse hovering
        Control* m_hover_elem;

        // text entry
        TextEntry* m_text_entry;

        // left-, middle-, and right-clicked elements
        Control* m_lclick_elem;
        Control* m_mclick_elem;
        Control* m_rclick_elem;

        // last click event
        sf::Time m_last_click_time;
        sf::Mouse::Button m_last_click_btn;

        // key pressed elements
        std::map<Key, Control*> m_keypressed_elems;

        class RootContainer : public FreeContainer {
        public:
            RootContainer(Window*);

        private:

            Window* m_window;
            Window* getWindow() const override;
        };

        RootContainer m_root;
        sf::RenderWindow m_sfwindow;

        friend class Context;
        friend class Control;
        friend class Element;
        friend class Draggable;
        friend class TextEntry;
    };

} // namespace ui
