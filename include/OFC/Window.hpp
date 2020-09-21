#pragma once

#include <OFC/DOM/FreeContainer.hpp>
#include <OFC/DOM/Control.hpp>
#include <OFC/DOM/TextEntry.hpp>

#include <OFC/Util/Vec2.hpp>

#include <OFC/Component/Root.hpp>

#include <SFML/Window.hpp>
#include <string>

namespace ofc::ui {

    // TODO: change cursors when mouse is over control, text entry, draggable, etc

    // TODO: add option to prevent elements from being considered for receiving input
    // For example, some child element which serves only to decorate a Control

    // TODO: SFML keypress events include information about modifiers keys that were
    // pressed at the time of the last keypress, such as alt, shift, and control.
    // It's probably more reliable to use these in onKeyDown than checking for
    // whether these keys are held (redundantly, thanks to left/right duplicate keys)

    
    class KeyboardCommand;

    class Window {
    public:
        Window(const Window&) = delete;
        Window(Window&&) = delete;
        Window& operator=(const Window&) = delete;
        Window& operator=(Window&&) = delete;
        ~Window();
        
        static Window& create(Root root, unsigned width = 600, unsigned height = 400, const String& title = "OFC App");

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

        // Command stays active until the returned object is destroyed.
        // Note: KeyboardCommand is an RAII type, moving it will keep the command active
        KeyboardCommand addKeyboardCommand(Key trigger, std::function<void()> callback);
        KeyboardCommand addKeyboardCommand(Key trigger, std::vector<Key> requiredKeys, std::function<void()> callback);

        // close the window, destroying it in the process
        // DO NOT use the object after calling close()
        void close();

        // the window's OS focus
        bool inFocus() const;
        void requestFocus();

    private:
        Window(unsigned width, unsigned height, const String& title, Root root);

        // process the window's event queue
        void processEvents();

        // update ui components
        void tick();

        // redraw the UI
        void redraw();

        dom::Control* findControlAt(vec2 p, const dom::Element* exclude = nullptr);

        // emulates the releasing of all held
        // keys and mouse buttons
        void releaseAllButtons();
        
        void handleMouseDown(sf::Mouse::Button btn, vec2 pos);
        void handleMouseUp(sf::Mouse::Button btn);

        void handleKeyDown(sf::Keyboard::Key key);
        void handleKeyUp(sf::Keyboard::Key key);

        void handleType(sf::Int32 unicode);

        void handleScroll(vec2 pos, vec2 scroll);

        void handleDrag();

        void handleHover(vec2 pos);

        bool handleCommand(Key);

        bool handleTextEntryKeyDown(Key);

        void transferResponseTo(dom::Control*);

        bool dropDraggable(dom::Draggable* d, vec2 pos);

        // Ensures that event listeners remain attached to
        // the element if it is temporarily removed and
        // then returned to the window.
        // After calling this function, there remains an
        // association between the window and the element
        // which maintains these event listeners. This
        // association is lost if the element is destroyed
        // or added to a different window.
        void softRemove(dom::Element*);

        // If the element has previously been softly removed,
        // returns event listeners as if the element had
        // never been removed. Otherwise, if the element has
        // not previously been softly removed, does nothing.
        void undoSoftRemove(dom::Element*);

        bool isSoftlyRemoved(const dom::Element*) const;

        // clean up all listeners for a window
        void hardRemove(dom::Element*);

        // Hard removes any softly removed elements which are no
        // longer part of the window.
        // Undoes the soft removal of any softly removed elements
        // which were returned to the window.
        void purgeRemovalQueue();

        // focus to a control, effectively calling onLoseFocus
        // on all controls from the currently one up to the common
        // ancestor, then calling onFocus on all controls down to
        // the new one.
        // passing null will remove the focus from all elements.
        void focusTo(dom::Control*);

        dom::Control* currentControl() const;

        void startDrag(dom::Draggable*, vec2);
        void stopDrag();
        dom::Draggable* currentDraggable();

        void startTyping(dom::TextEntry*);
        void stopTyping();
        dom::TextEntry* currentTextEntry();

        void addTransition(dom::Element* e, double duration, std::function<void(double)> fn, std::function<void()> onComplete);
        void removeTransitions(const dom::Element* e);

        void applyTransitions();


        void enqueueForUpdate(dom::Element*);
        void updateAllElements();
        void updateOneElement(dom::Element*);
        void cancelUpdate(const dom::Element*);

    private:

        /********** UI state **********/

        // focus
        dom::Control* m_focus_elem;

        // dragging
        dom::Draggable* m_drag_elem;
        vec2 m_drag_offset;

        // mouse hovering
        dom::Control* m_hover_elem;

        // text entry
        dom::TextEntry* m_text_entry;

        // left-, middle-, and right-clicked elements
        dom::Control* m_lclick_elem;
        dom::Control* m_mclick_elem;
        dom::Control* m_rclick_elem;
        bool m_lClickReleased;
        bool m_mClickReleased;
        bool m_rClickReleased;

        // last click event
        sf::Time m_last_click_time;
        sf::Mouse::Button m_last_click_btn;

        // elements which handled a key press
        std::map<Key, dom::Control*> m_keypressed_elems;

        dom::Control* m_currentEventResponder;

        struct Transition {
            dom::Element* element;
            double duration;
            std::function<void(double)> fn;
            std::function<void()> onComplete;
            sf::Time timeStamp;
        };

        std::vector<Transition> m_transitions;

        std::vector<dom::Element*> m_updateQueue;

        std::vector<dom::Element*> m_removalQueue;

        // registered keyboard commands
        
        struct KeyboardCommandSignal {
            KeyboardCommandSignal();
            ~KeyboardCommandSignal();

            KeyboardCommandSignal(KeyboardCommandSignal&&) = delete;
            KeyboardCommandSignal(const KeyboardCommandSignal&) = delete;
            KeyboardCommandSignal& operator=(KeyboardCommandSignal&&) = delete;
            KeyboardCommandSignal& operator=(const KeyboardCommandSignal&) = delete;

            Key trigger;
            std::vector<Key> requiredKeys;
            std::function<void()> callback;
            KeyboardCommand* connection;
        };

        std::vector<std::unique_ptr<KeyboardCommandSignal>> m_commands;

        friend class KeyboardCommand;

        Root m_root;
        std::unique_ptr<dom::Container> m_domRoot;
        sf::RenderWindow m_sfwindow;

        friend class dom::Element;
        friend class dom::Container;
        friend class dom::Control;
        friend class dom::Draggable;
        friend class dom::TextEntry;

        friend class ProgramContext;

        template<typename... ArgsT>
        friend dom::Control* propagate(Window*, dom::Control*, bool (dom::Control::*)(ArgsT...), ArgsT...);
    };

    class KeyboardCommand {
    public:
        KeyboardCommand() noexcept;
        KeyboardCommand(KeyboardCommand&&) noexcept;
        KeyboardCommand& operator=(KeyboardCommand&&) noexcept;
        ~KeyboardCommand();

        KeyboardCommand(const KeyboardCommand&) = delete;
        KeyboardCommand& operator=(const KeyboardCommand&) = delete;

        void reset();

    private:
        KeyboardCommand(Window*, Window::KeyboardCommandSignal*) noexcept;

        Window* m_window;
        Window::KeyboardCommandSignal* m_signal;

        friend class Window;
    };

} // namespace ofc::ui
