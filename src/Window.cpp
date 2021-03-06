#include <OFC/Window.hpp>

#include <OFC/ProgramContext.hpp>
#include <OFC/Component/Component.hpp>

#include <OFC/DOM/Draggable.hpp>

#include <cassert>

namespace ofc::ui {

    // Generic function for propagating an event through handler callbacks
    // Calls `handlerFn` on the element with the given arguments. If the
    // element returns `true`, that element has responded to the event, and
    // the element is returned. Otherwise, the process is repeated on its
    // ancestor controls, until one handles the event, or null is returned.
    template<typename... ArgsT>
    dom::Control* propagate(Window* self, dom::Control* elem, bool (dom::Control::* handlerFn)(ArgsT...), ArgsT... args){
        if (!elem){
            return nullptr;
        }

        // Pointer to the (optional) control that will receive the
        // corresponding end event (i.e. onKeyUp for responding to onKeyDown).
        // Controls may modify this while responding to an event by calling
        // transferEventResponseTo()
        self->m_currentEventResponder = elem;

        while (self->m_currentEventResponder){
            assert(self->m_currentEventResponder->getParentWindow() == self);
            const auto currentElem = elem;
            if ((self->m_currentEventResponder->*handlerFn)(args...)){
                if (self->m_currentEventResponder == nullptr){
                    return nullptr;
                } else if (self->m_currentEventResponder != currentElem){
                    assert(self->m_currentEventResponder->getParentWindow() == self);
                    return self->m_currentEventResponder;
                } else {
                    return self->m_currentEventResponder;
                }
            }
            self->m_currentEventResponder = self->m_currentEventResponder->getParentControl();
        }
        return nullptr;
    }

    Window::Window(unsigned width, unsigned height, const String& title, Root root) :
        m_sfwindow(),
        m_focus_elem(nullptr),
        m_drag_elem(nullptr),
        m_drag_offset({0.0f, 0.0f}),
        m_hover_elem(nullptr),
        m_text_entry(nullptr),
        m_lclick_elem(nullptr),
        m_mclick_elem(nullptr),
        m_rclick_elem(nullptr),
        m_lClickReleased(false),
        m_mClickReleased(false),
        m_rClickReleased(false),
        m_currentEventResponder(nullptr),
        m_last_click_time(),
        m_last_click_btn(),
        m_keypressed_elems(),
        m_root(std::move(root)),
        m_domRoot(nullptr) {

        sf::ContextSettings settings;
        settings.antialiasingLevel = 8;
        m_sfwindow.create(sf::VideoMode(width, height), title, sf::Style::Default, settings);
        m_sfwindow.setVerticalSyncEnabled(true);

        m_domRoot = m_root.mount(this);
        assert(m_domRoot);
        assert(m_domRoot->m_parentWindow == this);
        m_domRoot->requireDeepUpdate();
    }

    Window::~Window() {
        purgeRemovalQueue();
        m_root.unmount();
    }

    Window& Window::create(Root root, unsigned width, unsigned height, const String& title){
        // HACK because std::make_unique can't access private constructors
        auto pw = std::unique_ptr<Window>(new Window(width, height, title, std::move(root)));
        auto& wr = *pw;
        ProgramContext::get().addWindow(std::move(pw));
        return wr;
    }

    void Window::redraw(){
        m_sfwindow.clear(sf::Color::White);
        sf::View v;
        const auto screensize = getSize();
        v.reset({{0.0, 0.0}, screensize});
        m_sfwindow.setView(v);
        m_domRoot->setPos({0.0f, 0.0f});
        m_domRoot->setSize(getSize());
        updateAllElements();
        m_domRoot->render(m_sfwindow);
        m_sfwindow.display();
    }

    vec2 Window::getSize() const {
        auto s = m_sfwindow.getSize();
        return {static_cast<float>(s.x), static_cast<float>(s.y)};
    }

    void Window::setSize(vec2 s){
        m_sfwindow.setSize({static_cast<unsigned>(s.x), static_cast<unsigned>(s.y)});
    }

    vec2 Window::getPosition() const {
        auto p = m_sfwindow.getSize();
        return {static_cast<float>(p.x), static_cast<float>(p.y)};
    }

    void Window::setPosition(vec2 p){
        m_sfwindow.setPosition({static_cast<int>(p.x), static_cast<int>(p.y)});
    }

    vec2 Window::getMousePosition() const {
        const auto p = sf::Mouse::getPosition(m_sfwindow);
        return {static_cast<float>(p.x), static_cast<float>(p.y)};
    }

    void Window::setTitle(const String& t){
        m_sfwindow.setTitle(t);
    }

    void Window::setIcon(const sf::Image& img){
        const auto data = img.getPixelsPtr();
        if (!data){
            return;
        }
        const auto s = img.getSize();
        m_sfwindow.setIcon(s.x, s.y, data);
    }

    sf::Image Window::screenshot() const {
        const auto s = m_sfwindow.getSize();
        sf::Texture tex;
        tex.create(s.x, s.y);
        tex.update(m_sfwindow);
        return tex.copyToImage();
    }

    KeyboardCommand Window::addKeyboardCommand(Key trigger, std::function<void()> callback){
        return addKeyboardCommand(trigger, {}, std::move(callback));
    }

    KeyboardCommand Window::addKeyboardCommand(Key trigger, std::vector<ModifierKeys::KeyCode> requiredKeys, std::function<void()> callback){
        auto cmd = std::make_unique<KeyboardCommandSignal>();
        cmd->trigger = trigger;
        cmd->requiredKeys = std::move(requiredKeys);
        cmd->callback = std::move(callback);
        auto conn = KeyboardCommand(this, cmd.get());
        cmd->connection = &conn;
        m_commands.push_back(std::move(cmd));
        return conn;
    }

    void Window::close(){
        ProgramContext::get().removeWindow(this);
    }

    bool Window::inFocus() const {
        return m_sfwindow.hasFocus();
    }

    void Window::requestFocus(){
        m_sfwindow.requestFocus();
    }

    void Window::processEvents(){
        sf::Event event;
        while (m_sfwindow.pollEvent(event)){
            switch (event.type){
                case sf::Event::Closed: {
                    close();
                    return;
                }
                /* case sf::Event::Resized: {
                    
                } */
                case sf::Event::LostFocus: {
                    releaseAllButtons();
                    stopDrag();
                    break;
                }
                /* case sf::Event::GainedFocus: {

                } */
                case sf::Event::TextEntered: {
                    handleType(event.text.unicode);
                    break;
                }
                case sf::Event::KeyPressed: {
                    const auto mod = ModifierKeys(
                        event.key.alt,
                        event.key.control,
                        event.key.shift,
                        event.key.system
                    );
                    handleKeyDown(event.key.code, mod);
                    break;
                }
                case sf::Event::KeyReleased: {
                    handleKeyUp(event.key.code);
                    break;
                }
                case sf::Event::MouseWheelScrolled: {
                    auto delta = vec2{};
                    if (event.mouseWheelScroll.wheel == sf::Mouse::Wheel::HorizontalWheel){
                        delta.x = event.mouseWheelScroll.delta;
                    } else if (event.mouseWheelScroll.wheel == sf::Mouse::Wheel::VerticalWheel){
                        delta.y = event.mouseWheelScroll.delta;
                    }
                    const auto x = static_cast<float>(event.mouseWheelScroll.x);
                    const auto y = static_cast<float>(event.mouseWheelScroll.y);
                    handleScroll({x, y}, delta, modifierKeysFromKeyboard());
                    break;
                }
                case sf::Event::MouseButtonPressed: {
                    const auto x = static_cast<float>(event.mouseButton.x);
                    const auto y = static_cast<float>(event.mouseButton.y);
                    handleMouseDown(event.mouseButton.button, {x, y}, modifierKeysFromKeyboard());
                    break;
                }
                case sf::Event::MouseButtonReleased: {
                    handleMouseUp(event.mouseButton.button);
                    break;
                }
            }
        }
    }

    void Window::tick(){
        purgeRemovalQueue();
        handleDrag();
        handleHover(getMousePosition());
    }

    dom::Control* Window::findControlAt(vec2 p, const dom::Element* exclude){
        const auto hitElem = m_domRoot->findElementAt(p, exclude);
        if (!hitElem){
            return nullptr;
        }
        if (auto c = hitElem->toControl()){
            return c;
        }
        return hitElem->getParentControl();
    }

    void Window::releaseAllButtons(){
        if (m_lclick_elem && !m_lClickReleased){
            if (!isSoftlyRemoved(m_lclick_elem)){
                m_lclick_elem->onLeftRelease();
            }
            m_lclick_elem = nullptr;
        }
        if (m_mclick_elem && !m_mClickReleased){
            if (!isSoftlyRemoved(m_mclick_elem)){
                m_mclick_elem->onMiddleRelease();
            }
            m_mclick_elem = nullptr;
        }
        if (m_rclick_elem && !m_rClickReleased){
            if (!isSoftlyRemoved(m_rclick_elem)){
                m_rclick_elem->onRightRelease();
            }
            m_rclick_elem = nullptr;
        }
        for (auto& [key, ctrl] : m_keypressed_elems){
            if (!isSoftlyRemoved(ctrl)){
                ctrl->onKeyUp(key);
            }
        }
        m_keypressed_elems.clear();
    }

    void Window::handleMouseDown(sf::Mouse::Button btn, vec2 pos, ModifierKeys mod){
        const auto hitCtrl = findControlAt(pos);

        focusTo(hitCtrl);

        bool recent = (ProgramContext::get().getProgramTime() - m_last_click_time) <= ProgramContext::get().getDoubleClickTime();

        bool sameBtn = btn == m_last_click_btn;

        bool sameElem = false;
        if (btn == sf::Mouse::Left){
            sameElem = m_lclick_elem == hitCtrl;
        } else if (btn == sf::Mouse::Middle){
            sameElem = m_mclick_elem == hitCtrl;
        } else if (btn == sf::Mouse::Right){
            sameElem = m_rclick_elem == hitCtrl;
        }

        int numClicks = 1;
        if (recent && sameBtn && sameElem){
            numClicks = 2;

            // don't let it be double clicked again until after it gets single clicked again
            // achieved by faking an old timestamp
            m_last_click_time = ProgramContext::get().getProgramTime() - ProgramContext::get().getDoubleClickTime();
        } else {
            m_last_click_time = ProgramContext::get().getProgramTime();
        }

        if (btn == sf::Mouse::Left) {
            m_lclick_elem = propagate(this, hitCtrl, &dom::Control::onLeftClick, numClicks, mod);
            m_lClickReleased = false;
        } else if (btn == sf::Mouse::Middle) {
            m_mclick_elem = propagate(this, hitCtrl, &dom::Control::onMiddleClick, numClicks, mod);
            m_mClickReleased = false;
        } else if (btn == sf::Mouse::Right) {
            m_rclick_elem = propagate(this, hitCtrl, &dom::Control::onRightClick, numClicks, mod);
            m_rClickReleased = false;
        }
        
        m_last_click_btn = btn;
    }

    void Window::handleMouseUp(sf::Mouse::Button btn){
        if (btn == sf::Mouse::Left) {
            if (m_lclick_elem) {
                if (!isSoftlyRemoved(m_lclick_elem)){
                    m_lclick_elem->onLeftRelease();
                }
                m_lClickReleased = true;
            }
        } else if (btn == sf::Mouse::Middle) {
            if (m_mclick_elem) {
                if (!isSoftlyRemoved(m_mclick_elem)){
                    m_mclick_elem->onMiddleRelease();
                }
                m_mClickReleased = true;
            }
        } else if (btn == sf::Mouse::Right) {
            if (m_rclick_elem) {
                if (!isSoftlyRemoved(m_rclick_elem)){
                    m_rclick_elem->onRightRelease();
                }
                m_rClickReleased = true;
            }
        }
    }

    void Window::handleKeyDown(sf::Keyboard::Key key, ModifierKeys mod){
        if (handleTextEntryKeyDown(key, mod)){
            return;
        }

        if (m_text_entry && !isSoftlyRemoved(m_text_entry)) {
            return;
        }

        if (handleCommand(key)){
            return;
        }

        // if no command was found, send key stroke to the current element
        assert(!m_focus_elem || !isSoftlyRemoved(m_focus_elem));
        auto elem = propagate(this, m_focus_elem, &dom::Control::onKeyDown, key, mod);

        // and send key up to last element receiving same keystroke
        // in case of switching focus while key is held
        auto key_it = m_keypressed_elems.find(key);
        if (key_it != m_keypressed_elems.end()) {
            if (key_it->second && key_it->second != elem) {
                if (!isSoftlyRemoved(key_it->second)){
                    key_it->second->onKeyUp(key);
                }
                key_it->second = elem;
            }
        } else if (elem) {
            m_keypressed_elems[key] = elem;
        }

        // TODO: keyboard navigation
    }

    void Window::handleKeyUp(sf::Keyboard::Key key){
        auto it = m_keypressed_elems.find(key);
        if (it != m_keypressed_elems.end()) {
            auto e = it->second;
            assert(e);
            if (!isSoftlyRemoved(e)){
                e->onKeyUp(key);
            }
            m_keypressed_elems.erase(it);
        }
    }

    void Window::handleType(sf::Int32 unicode){
        if (m_text_entry && !isSoftlyRemoved(m_text_entry)){
            // Ignore any (extended) ASCII unprintable characters
            if ((unicode >= 32 && unicode < 127) || unicode > 161){
                m_text_entry->type(unicode);
            }
        }
    }

    void Window::handleScroll(vec2 pos, vec2 scroll, ModifierKeys mod){
        auto elem = findControlAt(pos);
        propagate(this, elem, &dom::Control::onScroll, scroll, mod);
    }

    void Window::handleDrag(){
        if (m_drag_elem && !isSoftlyRemoved(m_drag_elem)){
            const auto mousePos = getMousePosition();
            auto rootPos = vec2{};
            if (const auto c = m_drag_elem->getParentContainer()){
                rootPos = c->rootPos();
            }
            m_drag_elem->setPos(mousePos - rootPos - m_drag_offset);
            m_drag_elem->onDrag();
        }
    }

    void Window::handleHover(vec2 pos){
        auto newElem = findControlAt(pos, m_drag_elem);

        std::vector<dom::Control*> pathUp, pathDown;
        assert(!m_hover_elem || !isSoftlyRemoved(m_hover_elem));
        auto curr = m_hover_elem;
        while (curr){
            pathUp.push_back(curr);
            curr = curr->getParentControl();
        }

        curr = newElem;
        while (curr){
            pathDown.push_back(curr);
            curr = curr->getParentControl();
        }

        while (pathUp.size() > 0 && pathDown.size() > 0 && pathUp.front() == pathDown.front()){
            pathUp.erase(pathUp.begin());
            pathDown.erase(pathDown.begin());
        }

        for (auto it = pathUp.begin(); it != pathUp.end(); ++it) {
            (*it)->onMouseLeave(m_drag_elem);
        }

        for (auto it = pathDown.rbegin(); it != pathDown.rend(); ++it) {
            (*it)->onMouseEnter(m_drag_elem);
        }

        m_hover_elem = newElem;
    }

    bool Window::handleCommand(Key key){
        auto mod = modifierKeysFromKeyboard();

        KeyboardCommandSignal* bestCmd = nullptr;
        auto bestCmdHasExtraKeys = false;

        for (const auto& cmd : m_commands) {
            if (cmd->trigger == key) {
                if (!mod.hasAllKeysOf(cmd->requiredKeys)) {
                    continue;
                }
                const auto extraKeys = mod.hasDifferentKeysFrom(cmd->requiredKeys);
                if (bestCmd) {
                    if (bestCmdHasExtraKeys && !extraKeys) {
                        bestCmd = cmd.get();
                        bestCmdHasExtraKeys = false;
                    }
                } else {
                    bestCmd = cmd.get();
                    bestCmdHasExtraKeys = extraKeys;
                }
            }
        }

        if (bestCmd) {
            bestCmd->callback();
            return true;
        }
        return false;

        // Junk below
        // search for longest matching set of keys in registered commands
        /*size_t max = 0;
        KeyboardCommandSignal* best_cmd = nullptr;
        for (const auto& cmd : m_commands) {
            if (cmd->trigger == key) {
                bool match = true;
                for (int i = 0; i < cmd->requiredKeys.size() && match; i++) {
                    match = sf::Keyboard::isKeyPressed(cmd->requiredKeys[i]);
                }
                if (match && cmd->requiredKeys.size() >= max) {
                    max = cmd->requiredKeys.size();
                    best_cmd = cmd.get();
                }
            }
        }

        if (best_cmd) {
            // if one was found, invoke that command
            best_cmd->callback();
            return true;
        }
        return false;*/
    }

    bool Window::handleTextEntryKeyDown(Key key, ModifierKeys mod){
        if (!m_text_entry || isSoftlyRemoved(m_text_entry)){
            return false;
        }

        switch (key){
        case Key::Escape:
                m_text_entry->stopTyping();
                break;
            case Key::BackSpace:
                m_text_entry->handleBackspace(mod);
                break;
            case Key::Delete:
                m_text_entry->handleDelete(mod);
                break;
            case Key::Left:
                m_text_entry->handleLeft(mod);
                break;
            case Key::Right:
                m_text_entry->handleRight(mod);
                break;
            case Key::Home:
                m_text_entry->handleHome(mod);
                break;
            case Key::End:
                m_text_entry->handleEnd(mod);
                break;
            case Key::Enter:
                m_text_entry->handleReturn();
                m_text_entry->stopTyping();
                break;
            case Key::Insert:
                m_text_entry->handleInsert();
                break;
            case Key::A:
                if (mod.ctrl()){
                    m_text_entry->handleSelectAll();
                    break;
                } else {
                    return false;
                }
            case Key::C:
                if (mod.ctrl()){
                    m_text_entry->handleCopy();
                    break;
                } else {
                    return false;
                }
            case Key::X:
                if (mod.ctrl()){
                    m_text_entry->handleCut();
                    break;
                } else {
                    return false;
                }
            case Key::V:
                if (mod.ctrl()){
                    m_text_entry->handlePaste();
                    break;
                } else {
                    return false;
                }
            default:
                return false;
            // NOTE: typing of actual characters is handled by the text entered event
        }

        return true;
    }

    void Window::transferResponseTo(dom::Control* c){
        assert(c);
        assert(c->getParentWindow() == this);
        m_currentEventResponder = c;
    }

    bool Window::dropDraggable(dom::Draggable* d, vec2 pos){
        assert(d);
        auto c = findControlAt(pos, d);
        while (c){
            if (c->onDrop(d)){
                return true;
            }
            c = c->getParentControl();
        }
        return false;
    }

    ModifierKeys Window::modifierKeysFromKeyboard() const noexcept {
        const bool alt =
            sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::RAlt);
        const bool ctrl =
            sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);
        const bool shift =
            sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
        const bool system =
            sf::Keyboard::isKeyPressed(sf::Keyboard::LSystem) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::RSystem);
        return ModifierKeys(alt, ctrl, shift, system);
    }

    void Window::softRemove(dom::Element* e){
        assert(e->getParentWindow() == this);
        assert(e->m_previousWindow == nullptr);
        assert(count(begin(m_removalQueue), end(m_removalQueue), e) == 0);
        assert(!isSoftlyRemoved(e));
        e->m_previousWindow = this;
        m_removalQueue.push_back(e);
        cancelUpdate(e);
    }

    void Window::undoSoftRemove(dom::Element* e){
        assert(e->getParentWindow() == nullptr);
        assert(e->m_previousWindow == this || e->m_previousWindow == nullptr);
        assert(isSoftlyRemoved(e));
        if (e->m_previousWindow == this){
            assert(count(begin(m_removalQueue), end(m_removalQueue), e) == 1);
            e->m_previousWindow = nullptr;
            auto it = find(begin(m_removalQueue), end(m_removalQueue), e);
            assert(it != end(m_removalQueue));
            m_removalQueue.erase(it);
            assert(count(begin(m_removalQueue), end(m_removalQueue), e) == 0);
        }
        e->m_previousWindow = nullptr;
    }

    bool Window::isSoftlyRemoved(const dom::Element* e) const {
        assert(e);
        assert([&]{
            auto c = count(begin(m_removalQueue), end(m_removalQueue), e);
            if (e->m_previousWindow == this){
                return c == 1;
            } else {
                return c == 0 && e->m_previousWindow == nullptr;
            }
        }());
        return e->m_previousWindow == this;
    }

    void Window::hardRemove(dom::Element* e){
        // NOTE: this function will be called during the
        // destructor of Element. Do not call any virtual functions.
        assert(e);

        auto parentControl = e->getParentControl();

        const auto cleanup = [&](const dom::Element* elem){
            if (elem == m_focus_elem){
                m_focus_elem = parentControl;
            }
            if (elem == m_drag_elem){
                stopDrag();
            }
            if (elem == m_hover_elem){
                m_hover_elem = parentControl;
            }
            if (elem == m_text_entry){
                stopTyping();
            }
            if (elem == m_lclick_elem){
                m_lclick_elem = nullptr;
            }
            if (elem == m_mclick_elem){
                m_lclick_elem = nullptr;
            }
            if (elem == m_rclick_elem){
                m_lclick_elem = nullptr;
            }
            if (elem == m_currentEventResponder){
                m_currentEventResponder = nullptr;
            }
            for (auto it = m_keypressed_elems.begin(), end = m_keypressed_elems.end(); it != end;){
                const auto& [key, ctrl] = *it;
                if (elem == ctrl){
                    m_keypressed_elems.erase(it++);
                    break;
                } else {
                    ++it;
                }
            }
            cancelUpdate(elem);
        };

        std::function<void(const dom::Element*)> cleanupAll = [&](const dom::Element* elem){
            if (auto cont = elem->toContainer()){
                for (auto child : cont->children()){
                    cleanupAll(child);
                }
            }
            cleanup(elem);
        };

        assert(e->getParentWindow() == this || e->getParentWindow() == nullptr);
        cleanupAll(e);
        
        assert(e->m_previousWindow == nullptr || e->m_previousWindow == this);
        if (e->m_previousWindow == this){
            assert(count(begin(m_removalQueue), end(m_removalQueue), e) == 1);
            e->m_previousWindow = nullptr;
            auto it = find(begin(m_removalQueue), end(m_removalQueue), e);
            assert(it != end(m_removalQueue));
            m_removalQueue.erase(it);
            assert(count(begin(m_removalQueue), end(m_removalQueue), e) == 0);
        }
        e->m_previousWindow = nullptr;
    }

    void Window::purgeRemovalQueue(){
        while (!m_removalQueue.empty()){
            auto e = m_removalQueue.back();
            assert(e);
            // If the element is still here: undo soft removal
            // If the element is not part of this window: hard removal
            // If the element was destroyed: Already hard removed, it won't be in the queue.
            if (e->getParentWindow() == this){
                undoSoftRemove(e);
            } else {
                hardRemove(e);
            }
            assert(count(begin(m_removalQueue), end(m_removalQueue), e) == 0);
        }
    }

    void Window::focusTo(dom::Control* control){
        assert(!control || control->getParentWindow() == this);
        assert(!control || !isSoftlyRemoved(control));

        assert(!m_focus_elem || !isSoftlyRemoved(m_focus_elem));
        auto prev = m_focus_elem;

        std::vector<dom::Control*> pathUp, pathDown;

        auto curr = m_focus_elem;
        if (curr && curr->getParentWindow() == this){
            while (curr){
                pathUp.push_back(curr);
                curr = curr->getParentControl();
            }
        }

        curr = control;
        while (curr){
            pathDown.push_back(curr);
            curr = curr->getParentControl();
        }

        while (pathUp.size() > 0 && pathDown.size() > 0 && pathUp.back() == pathDown.back()){
            pathUp.pop_back();
            pathDown.pop_back();
        }

        for (auto it = pathUp.rbegin(), itEnd = pathUp.rend(); it != itEnd; ++it){
            (*it)->onLoseFocus();
            if (m_focus_elem != prev){
                return;
            }
        }

        for (auto& c : pathDown){
            c->onGainFocus();
            if (m_focus_elem != prev){
                return;
            }
        }

        m_focus_elem = control;
    }

    dom::Control* Window::currentControl() const {
        assert(!m_focus_elem || !isSoftlyRemoved(m_focus_elem));
        return m_focus_elem;
    }

    void Window::startDrag(dom::Draggable* d, vec2 o){
        assert(d);
        assert(d->getParentWindow() == this);
        assert(!isSoftlyRemoved(d));
        m_drag_elem = d;
        m_drag_offset = o;
    }

    void Window::stopDrag(){
        m_drag_elem = nullptr;
    }

    dom::Draggable* Window::currentDraggable(){
        assert(!m_drag_elem || !isSoftlyRemoved(m_drag_elem));
        return m_drag_elem;
    }

    void Window::startTyping(dom::TextEntry* te){
        assert(te);
        m_text_entry = te;
        focusTo(te);
    }

    void Window::stopTyping(){
        m_text_entry = nullptr;
    }

    dom::TextEntry* Window::currentTextEntry(){
        return m_text_entry;
    }

    void Window::enqueueForUpdate(dom::Element* elem){
        if (std::find(m_updateQueue.begin(), m_updateQueue.end(), elem) != m_updateQueue.end()){
            return;
        }
        m_updateQueue.push_back(elem);
        assert(std::count(m_updateQueue.begin(), m_updateQueue.end(), elem) == 1);
    }

    void Window::updateAllElements(){
        while (!m_updateQueue.empty()){
            updateOneElement(m_updateQueue.front());
        }
    }

    void Window::updateOneElement(dom::Element* elem){
        // NOTE: the size is being accessed directly instead of through
        // get/setSize() to avoid marking the element dirty again

        const auto maxUpdates = std::size_t{10};
        for (std::size_t i = 0; i < maxUpdates; ++i){

            assert(elem);
            assert(!elem->m_isUpdating);
            elem->m_isUpdating = true;

            // Remove the element from the queue
            {
                m_updateQueue.erase(std::remove(
                    m_updateQueue.begin(),
                    m_updateQueue.end(),
                    elem
                ), m_updateQueue.end());
            }

            // Get the element's original size
            const auto prevSize = elem->m_parent ?
                elem->m_parent->getPreviousSize(elem) :
                std::optional<vec2>{};

            // Retrieve the element's available size
            const auto availSize = [&](){
                if (auto c = elem->toContainer(); c && c->shrink()){
                    return std::optional{vec2{0.0f, 0.0f}};
                } else if (auto p = elem->getParentContainer()){
                    return p->getAvailableSize(elem);
                } else {
                    assert(elem == m_domRoot.get());
                    return std::optional{getSize()};
                }
            }();

            // Set the element's size to be the available size
            if (availSize){
                elem->m_size = *availSize;
            }
            // Limit the element's size according to its minimum and maximum size
            elem->m_size.x = std::clamp(elem->m_size.x, elem->m_minsize.x, elem->m_maxsize.x);
            elem->m_size.y = std::clamp(elem->m_size.y, elem->m_minsize.y, elem->m_maxsize.y);

            // Tell the element to update its contents and get the size it actually needs
            const auto actualRequiredSize = elem->update();
            
            // Let the container know the required size (which may differ from the final size)
            if (auto p = elem->getParentContainer()){
                p->setRequiredSize(
                    elem,
                    {
                        std::clamp(actualRequiredSize.x, elem->m_minsize.x, elem->m_maxsize.x),
                        std::clamp(actualRequiredSize.y, elem->m_minsize.y, elem->m_maxsize.y)
                    }
                );
            }

            if (!availSize){
                elem->m_size = actualRequiredSize;
            } else {
                elem->m_size.x = std::max(elem->m_size.x, actualRequiredSize.x);
                elem->m_size.y = std::max(elem->m_size.y, actualRequiredSize.y);
                if (actualRequiredSize.x > availSize->x || actualRequiredSize.y > availSize->y) {
                    if (auto p = elem->getParentContainer()){
                        p->requireUpdate();
                    }
                }
            }

            // Limit the element's size according to its minimum and maximum size
            elem->m_size.x = std::clamp(elem->m_size.x, elem->m_minsize.x, elem->m_maxsize.x);
            elem->m_size.y = std::clamp(elem->m_size.y, elem->m_minsize.y, elem->m_maxsize.y);

            // mark the element as clean
            elem->m_needs_update = false;

            if (auto c = elem->toContainer()){
                // cache the element's previous sizes to allow efficient rerendering decisions
                // (see getPreviousSize() above)
                c->updatePreviousSizes();
            }

            elem->m_isUpdating = false;

            // If the element's size changed from its previous size, or if the parent
            // is dirty, update it too
            const auto sizeChanged = 
                !prevSize.has_value() || (
                    std::abs(elem->m_size.x - prevSize->x) > 1e-6 ||
                    std::abs(elem->m_size.y - prevSize->y) > 1e-6
                );
        
            const auto couldUseLessSpace = 
                (actualRequiredSize.x < elem->m_size.x) ||
                (actualRequiredSize.y < elem->m_size.y);

            if (auto p = elem->m_parent){
                p->updatePreviousSizes(elem);
            }

            if (sizeChanged){
                elem->onResize();
            }

            // If the parent container possibly needs to update, make sure it gets updated soon
            if (elem->m_parent && (sizeChanged || couldUseLessSpace)){
                if (!elem->m_parent->m_isUpdating){
                    if (std::find(m_updateQueue.begin(), m_updateQueue.end(), elem->m_parent) == m_updateQueue.end()){
                        enqueueForUpdate(elem->m_parent);
                        elem->m_parent->m_needs_update = true;
                    }
                }
            }

            // If the element is still up-to-date after updating its parents,
            // stop right there
            if (!elem->m_needs_update){
                return;
            }
        }
        throw std::runtime_error("Exceeded maximum updated count");
    }

    void Window::cancelUpdate(const dom::Element* elem){
        assert(elem);
        assert(!elem->m_isUpdating);
        const auto isDecendent = [&](const dom::Element* e){
            const std::function<bool(const dom::Element*)> impl = [&](const dom::Element* x){
                if (elem == x){
                    return true;
                }
                if (auto c = x->getParentContainer()){
                    return impl(c);
                }
                return false;
            };
            return impl(e);
        };
        m_updateQueue.erase(std::remove_if(
            m_updateQueue.begin(),
            m_updateQueue.end(),
            isDecendent
        ), m_updateQueue.end());
    }

    KeyboardCommand::KeyboardCommand() noexcept
        : m_window(nullptr)
        , m_signal(nullptr) {

    }

    KeyboardCommand::KeyboardCommand(Window* w, Window::KeyboardCommandSignal* s) noexcept
        : m_window(w)
        , m_signal(s) {

    }

    KeyboardCommand::KeyboardCommand(KeyboardCommand&& c) noexcept
        : m_window(std::exchange(c.m_window, nullptr))
        , m_signal(std::exchange(c.m_signal, nullptr)) {
        if (m_signal) {
            m_signal->connection = this;
        }
    }

    KeyboardCommand& KeyboardCommand::operator=(KeyboardCommand&& c) noexcept {
        reset();
        m_window = std::exchange(c.m_window, nullptr);
        m_signal = std::exchange(c.m_signal, nullptr);
        if (m_signal) {
            m_signal->connection = this;
        }
        return *this;
    }

    KeyboardCommand::~KeyboardCommand(){
        reset();
    }

    void KeyboardCommand::reset(){
        assert(!!m_window == !!m_signal);
        if (m_window) {
            assert(m_signal);
            assert(m_signal->connection == this);
            m_signal->connection = nullptr;
            auto& cmds = m_window->m_commands;
            const auto match = [&](const std::unique_ptr<Window::KeyboardCommandSignal>& kcs) {
                return kcs.get() == m_signal;
            };
            assert(count_if(begin(cmds), end(cmds), match) == 1);
            cmds.erase(remove_if(begin(cmds), end(cmds), match ), end(cmds));
        }
        m_window = nullptr;
        m_signal = nullptr;
    }

    Window::KeyboardCommandSignal::KeyboardCommandSignal()
        : trigger(Key::Unknown)
        , requiredKeys{}
        , callback{}
        , connection(nullptr) {

    }

    Window::KeyboardCommandSignal::~KeyboardCommandSignal(){
        if (connection) {
            assert(connection->m_window);
            assert(connection->m_signal == this);
            connection->m_window = nullptr;
            connection->m_signal = nullptr;
            connection = nullptr;
        }
    }

} // namespace ofc::ui
