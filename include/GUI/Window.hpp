#pragma once

#include <GUI/FreeContainer.hpp>
#include <GUI/Control.hpp>
#include <GUI/TextEntry.hpp>

#include <SFML/Window.hpp>
#include <string>

namespace ui {

    // TODO: change cursors when mouse is over control, text entry, draggable, etc

    class Window {
    public:
        Window(const Window&) = delete;
        Window(Window&&) = delete;
        Window& operator=(const Window&) = delete;
        Window& operator=(Window&&) = delete;
        ~Window() = default;
        
        static Window& create(unsigned width, unsigned height, const String& title);

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

        void addKeyboardCommand(Key trigger, std::function<void()> callback);
        void addKeyboardCommand(Key trigger, std::vector<Key> requiredKeys, std::function<void()> callback);

        // close the window, destroying it in the process
        // DO NOT use the object after calling close()
        void close();

        // the window's OS focus
        bool inFocus() const;
        void requestFocus();

        void setRoot(std::unique_ptr<Container>);

        template<typename T, typename... Args>
        T& setRoot(Args&&... args);

    private:
        Window(unsigned width, unsigned height, const String& title);

        // process the window's event queue
        void processEvents();

        // update ui components
        void tick();

        // redraw the UI
        void redraw();

        Control* findControlAt(vec2 p, const Element* exclude = nullptr);

        // emulates the releasing of all held
        // keys and mouse buttons
        void releaseAllButtons();
        
        void handleMouseDown(sf::Mouse::Button btn, vec2 pos);
        void handleMouseUp(sf::Mouse::Button btn, vec2 pos);

        void handleKeyDown(sf::Keyboard::Key key);
        void handleKeyUp(sf::Keyboard::Key key);

        void handleType(sf::Int32 unicode);

        void handleScroll(vec2 pos, vec2 scroll);

        void handleDrag();

        void handleHover(vec2 pos);

        bool handleCommand(Key);

        bool handleTextEntryKeyDown(Key);

        void transferResponseTo(Control*);

        void dropDraggable(Draggable* d, vec2 pos);

        // clean up all listeners for a window
        void onRemoveElement(Element*);

        // focus to a control, effectively calling onLoseFocus
        // on all controls from the currently one up to the common
        // ancestor, then calling onFocus on all controls down to
        // the new one.
        // passing null will remove the focus from all elements.
        void focusTo(Control*);

        Control* currentControl() const;

        void startDrag(Draggable*, vec2);
        void stopDrag();
        Draggable* currentDraggable();

        void startTyping(TextEntry*);
        void stopTyping();
        TextEntry* currentTextEntry();

        void addTransition(Element* e, double duration, std::function<void(double)> fn, std::function<void()> onComplete);
        void removeTransitions(const Element* e);

        void applyTransitions();


        void enqueueForUpdate(Element*);
        void updateAllElements();
        void updateOneElement(Element*);
        void cancelUpdate(const Element*);

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

        // elements which handled a key press
        std::map<Key, Control*> m_keypressed_elems;

        Control* m_currentEventResponder;

        struct Transition {
            Element* element;
            double duration;
            std::function<void(double)> fn;
            std::function<void()> onComplete;
            sf::Time timeStamp;
        };

        std::vector<Transition> m_transitions;

        std::vector<Element*> m_updateQueue;

        // registered keyboard commands
        // TODO: move this to context?
        std::map<std::pair<Key, std::vector<Key>>, std::function<void()>> m_commands;

        std::unique_ptr<Container> m_root;
        sf::RenderWindow m_sfwindow;

        friend class Element;
        friend class Container;
        friend class Context;
        friend class Control;
        friend class Draggable;
        friend class TextEntry;

        template<typename... ArgsT>
        friend Control* propagate(Window*, Control*, bool (Control::*)(ArgsT...), ArgsT...);
    };

    // template definitions

    template<typename T, typename... Args>
    inline T& Window::setRoot(Args&&... args){
        static_assert(std::is_base_of_v<Container, T>, "T must derive from ui::Container");
        auto uptr = std::make_unique<T>(std::forward<Args>(args)...);
        T& ret = *uptr;
        m_root = std::move(uptr);
        m_root->m_parentWindow = this;
        m_root->requireDeepUpdate();
        return ret;
    }

} // namespace ui
