#include <GUI/Window.hpp>

#include <GUI/Context.hpp>

#include <cassert>

namespace ui {

    namespace {
        // Generic function for propagating an event through handler callbacks
        // Calls `handlerFn` on the element with the given arguments. If the
        // element returns `true`, that element has responded to the event, and
        // the element is returned. Otherwise, the process is repeated on its
        // ancestor controls, until one handles the event, or null is returned.
        template<typename... ArgsT>
        Control* propagate(Control* elem, bool (Control::* handlerFn)(ArgsT...), ArgsT... args){
            while (elem){
                if ((elem->*handlerFn)(args...)){
                    return elem;
                }
                elem = elem->getParentControl();
            }
            return nullptr;
        }
    }

    Window::Window(unsigned width, unsigned height, const String& title) :
        m_sfwindow(),
        m_focus_elem(nullptr),
        m_drag_elem(nullptr),
        m_drag_offset({0.0f, 0.0f}),
        m_hover_elem(nullptr),
        m_text_entry(nullptr),
        m_lclick_elem(nullptr),
        m_mclick_elem(nullptr),
        m_rclick_elem(nullptr),
        m_last_click_time(),
        m_last_click_btn(),
        m_keypressed_elems(),
        m_root(std::make_unique<Container>()) {

        m_sfwindow.create(sf::VideoMode(width, height), title);
        m_sfwindow.setVerticalSyncEnabled(true);
    }

    Window& Window::create(unsigned width, unsigned height, const String& title){
        auto pw = std::unique_ptr<Window>(new Window(width, height, title));
        auto& wr = *pw;
        Context::get().addWindow(std::move(pw));
        return wr;
    }

    void Window::redraw(){
        m_sfwindow.clear(sf::Color::White);
        sf::View v;
        const auto screensize = getSize();
        v.reset({{0.0, 0.0}, screensize});
        m_sfwindow.setView(v);
        m_root->setSize(getSize());
        updateAllElements();
        m_root->render(m_sfwindow);
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

    void Window::close(){
        Context::get().removeWindow(this);
    }

    bool Window::inFocus() const {
        return m_sfwindow.hasFocus();
    }

    void Window::requestFocus(){
        m_sfwindow.requestFocus();
    }

    void Window::setRoot(std::unique_ptr<Container> c){
        m_root = std::move(c);
        m_root->m_parentWindow = this;
        m_root->requireDeepUpdate();
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
                    if (m_text_entry){
                        m_text_entry->type(event.text.unicode);
                    }
                    break;
                }
                case sf::Event::KeyPressed: {
                    if (m_focus_elem){
                        m_focus_elem->onKeyDown(event.key.code);
                    }
                    break;
                }
                case sf::Event::KeyReleased: {
                    if (m_focus_elem){
                        m_focus_elem->onKeyUp(event.key.code);
                    }
                    break;
                }
                case sf::Event::MouseWheelScrolled: {
                    auto delta = vec2{};
                    if (event.mouseWheelScroll.wheel == sf::Mouse::Wheel::HorizontalWheel){
                        delta.x = event.mouseWheelScroll.delta;
                    } else if (event.mouseWheelScroll.wheel == sf::Mouse::Wheel::VerticalWheel){
                        delta.y = event.mouseWheelScroll.delta;
                    }
                    const auto x = static_cast<float>(event.mouseButton.x);
                    const auto y = static_cast<float>(event.mouseButton.y);
                    handleScroll({x, y}, delta);
                    break;
                }
                case sf::Event::MouseButtonPressed: {
                    const auto x = static_cast<float>(event.mouseButton.x);
                    const auto y = static_cast<float>(event.mouseButton.y);
                    handleMouseDown(event.mouseButton.button, {x, y});
                    break;
                }
                case sf::Event::MouseButtonReleased: {
                    const auto x = static_cast<float>(event.mouseButton.x);
                    const auto y = static_cast<float>(event.mouseButton.y);
                    handleMouseUp(event.mouseButton.button, {x, y});
                    break;
                }
            }
        }
    }

    void Window::tick(){
        handleDrag();
        handleHover(getMousePosition());
        applyTransitions();
    }

    Control* Window::findControlAt(vec2 p){
        const auto hitElem = m_root->findElementAt(p);
        if (!hitElem){
            return nullptr;
        }
        if (auto c = hitElem->toControl()){
            return c;
        }
        return hitElem->getParentControl();
    }

    void Window::releaseAllButtons(){
        if (m_lclick_elem){
            m_lclick_elem->onLeftRelease();
            m_lclick_elem = nullptr;
        }
        if (m_mclick_elem){
            m_mclick_elem->onMiddleRelease();
            m_mclick_elem = nullptr;
        }
        if (m_rclick_elem){
            m_rclick_elem->onRightRelease();
            m_rclick_elem = nullptr;
        }
        for (auto& [key, ctrl] : m_keypressed_elems){
            ctrl->onKeyUp(key);
        }
        m_keypressed_elems.clear();
    }

    void Window::handleMouseDown(sf::Mouse::Button btn, vec2 pos){
        const auto hitCtrl = findControlAt(pos);

        bool recent = (Context::get().getProgramTime() - m_last_click_time) <= Context::get().getDoubleClickTime();

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
			m_last_click_time = Context::get().getProgramTime() - Context::get().getDoubleClickTime();
        } else {
            focusTo(hitCtrl);
            m_last_click_time = Context::get().getProgramTime();
        }

        if (btn == sf::Mouse::Left) {
			m_lclick_elem = propagate(hitCtrl, &Control::onLeftClick, numClicks);
		} else if (btn == sf::Mouse::Middle) {
			m_mclick_elem = propagate(hitCtrl, &Control::onMiddleClick, numClicks);
		} else if (btn == sf::Mouse::Middle) {
			m_rclick_elem = propagate(hitCtrl, &Control::onMiddleClick, numClicks);
		}

        m_last_click_btn = btn;
    }

    void Window::handleMouseUp(sf::Mouse::Button btn, vec2 pos){
        if (btn == sf::Mouse::Left) {
			if (m_lclick_elem) {
				m_lclick_elem->onLeftRelease();
			}
		} else if (btn == sf::Mouse::Middle) {
			if (m_mclick_elem) {
				m_mclick_elem->onRightRelease();
			}
		} else if (btn == sf::Mouse::Right) {
			if (m_rclick_elem) {
				m_rclick_elem->onRightRelease();
			}
		}
    }

    void Window::handleKeyDown(sf::Keyboard::Key key){
        // search for longest matching set of keys in registered commands
		size_t max = 0;
		auto current_cmd = m_commands.end();
		for (auto cmd_it = m_commands.begin(), end = m_commands.end(); cmd_it != end; ++cmd_it) {
			if (cmd_it->first.first == key) {
				bool match = true;
				for (int i = 0; i < cmd_it->first.second.size() && match; i++) {
					match = sf::Keyboard::isKeyPressed(cmd_it->first.second[i]);
				}
				if (match && cmd_it->first.second.size() >= max) {
					max = cmd_it->first.second.size();
					current_cmd = cmd_it;
				}
			}
		}

		if (current_cmd != m_commands.end()) {
			// if one was found, invoke that command
			current_cmd->second();
			return;
		}

		// if no command was found, send key stroke to the current element
		auto elem = propagate(m_focus_elem, &Control::onKeyDown, key);
		// and send key up to last element receiving same keystroke
		// in case of switching focus while key is held
		auto key_it = m_keypressed_elems.find(key);
		if (key_it != m_keypressed_elems.end()) {
			if (key_it->second && key_it->second != elem) {
				key_it->second->onKeyUp(key);
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
			assert(it->second);
		    it->second->onKeyUp(key);
			m_keypressed_elems.erase(it);
		}
    }

    void Window::handleScroll(vec2 pos, vec2 scroll){
        auto elem = findControlAt(pos);
        propagate(elem, &Control::onScroll, scroll);
    }

    void Window::handleDrag(){
        if (m_drag_elem){
            const auto mousePos = getMousePosition();
            auto rootPos = vec2{};
            if (const auto c = m_drag_elem->getParentContainer()){
                rootPos = c->rootPos();
            }
            // TODO: why not setPos()?
            m_drag_elem->m_position = mousePos - rootPos - m_drag_offset;
            m_drag_elem->onDrag();
        }
    }

    void Window::handleHover(vec2 pos){
        auto newElem = findControlAt(pos);
        if (newElem == m_hover_elem){
            return;
        }

        std::vector<Control*> pathUp, pathDown;
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

        for (auto it = pathUp.rbegin(), itEnd = pathUp.rend(); it != itEnd; ++it){
            (*it)->onMouseOut();
        }

        for (auto& c : pathDown){
            c->onMouseOver();
        }

        m_hover_elem = newElem;

        if (m_drag_elem){
            propagate(newElem, &Control::onHover, m_drag_elem);
        }
    }

    void Window::onRemoveElement(Element* elem){
        // NOTE: this function will be called during the
        // destructor of Element. Do not call any virtual functions.
        if (!elem){
            return;
        }
        if (elem == m_focus_elem){
            m_focus_elem = m_focus_elem->getParentControl();
        }
        if (elem == m_drag_elem){
            m_drag_elem->stopDrag();
        }
        if (elem == m_hover_elem){
            m_hover_elem = m_hover_elem->getParentControl();
            if (m_hover_elem){
                m_drag_offset += elem->rootPos() - m_hover_elem->rootPos();
            }
        }
        if (elem == m_text_entry){
            m_text_entry->stopTyping();
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
        for (auto it = m_keypressed_elems.begin(), end = m_keypressed_elems.end(); it != end;){
            const auto& [key, ctrl] = *it;
            if (elem == ctrl){
                m_keypressed_elems.erase(it++);
                break;
            } else {
                ++it;
            }
        }
        m_updateQueue.erase(std::remove(
            m_updateQueue.begin(),
            m_updateQueue.end(),
        elem), m_updateQueue.end());
    }

    void Window::focusTo(Control* control){
        auto prev = m_focus_elem;

        std::vector<Control*> pathUp, pathDown;
        auto curr = m_focus_elem;
        while (curr){
            pathUp.push_back(curr);
            curr = curr->getParentControl();
        }

        curr = control;
        while (curr){
            pathDown.push_back(curr);
            curr = curr->getParentControl();
        }

        while (pathUp.size() > 0 && pathDown.size() > 0 && pathUp.front() == pathDown.front()){
            pathUp.erase(pathUp.begin());
            pathDown.erase(pathDown.begin());
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

    void Window::startDrag(Draggable* d, vec2 o){
        m_drag_elem = d;
        m_drag_offset = o;
    }

    void Window::stopDrag(){
        m_drag_elem = nullptr;
    }

    Draggable* Window::currentDraggable(){
        return m_drag_elem;
    }

    void Window::startTyping(TextEntry* te){
        m_text_entry = te;
    }

    void Window::stopTyping(){
        m_text_entry = nullptr;
    }

    TextEntry* Window::currentTextEntry(){
        return m_text_entry;
    }

    void Window::addTransition(Element* e, double duration, std::function<void(double)> fn, std::function<void()> onComplete){
        m_transitions.push_back({
            e,
            duration,
            std::move(fn),
            std::move(onComplete),
            Context::get().getProgramTime()
        });
    }

    void Window::removeTransitions(const Element* e){
        m_transitions.erase(std::remove_if(
            m_transitions.begin(),
            m_transitions.end(),
            [&](const Transition& t){
                return t.element == e;
            }
        ), m_transitions.end());
    }

    void Window::applyTransitions(){
        const auto now = Context::get().getProgramTime();
        for (auto it = m_transitions.begin(); it != m_transitions.end();){
            const auto t = (now - it->timeStamp).asSeconds() / it->duration;
            if (t > 1.0){
                it->fn(1.0);
                it->on_complete();
                it = m_transitions.erase(it);
            } else {
                it->fn(t);
            }
        }
    }

    void Window::enqueueForUpdate(Element* elem){
        if (std::find(m_updateQueue.begin(), m_updateQueue.end(), elem) != m_updateQueue.end()){
            return;
        }
        m_updateQueue.push_back(elem);
    }

    void Window::updateAllElements(){
        while (!m_updateQueue.empty()){
            updateOneElement(m_updateQueue.front());
        }
    }

    void Window::updateOneElement(Element* elem){
        // NOTE: the size is being accessed directly instead of through
        // get/setSize() to avoid adding the element to the queue again

        assert(elem);
        assert(!elem->m_isUpdating);
        elem->m_isUpdating = true;

        // Remove the element from the queue
        {
            auto it = std::find(m_updateQueue.begin(), m_updateQueue.end(), elem);
            assert(it != m_updateQueue.end());
            m_updateQueue.erase(it);
        }

        // Get the element's original size
        const auto prevSize = elem->m_parent ?
            elem->m_parent->getPreviousSize(elem) :
            elem->m_size;

        // Retrieve the element's available size
        const auto availSize = [&](){
            if (auto p = elem->getParentContainer()){
                return p->getAvailableSize(elem);
            } else {
                assert(elem == m_root.get());
                return std::optional{getSize()};
            }
        }();

        // Set the element's size to be the available size
        if (availSize){
            elem->m_size = *availSize;
        }

        // Tell the element to update its contents and get the size it actually needs
        const auto actualSize = elem->update();

        if (!availSize){
            elem->m_size = actualSize;
        }

        // Limit the element's size according to its minimum and maximum size
        elem->m_size.x = std::clamp(elem->m_size.x, elem->m_minsize.x, elem->m_maxsize.x);
        elem->m_size.y = std::clamp(elem->m_size.y, elem->m_minsize.y, elem->m_maxsize.y);

        // mark the element as clean
        elem->m_needs_update = false;

        // cache the element's previous sizes to allow efficient rerendering decisions
        // (see getPreviousSize() above)
        if (auto c = elem->toContainer()){
            c->updatePreviousSizes();
        }

        elem->m_isUpdating = false;

        // If the element's size changed from its previous size, or if the parent
        // is dirty, update it too
        const auto sizeChanged = 
            std::abs(elem->m_size.x - prevSize.x) > 1e-6 ||
            std::abs(elem->m_size.y - prevSize.y) > 1e-6;
        
        if (sizeChanged){
            elem->onResize();
        }

        if (elem->m_parent && (elem->m_parent->m_needs_update || sizeChanged)){
            elem->m_parent->requireUpdate();
            elem->m_parent->forceUpdate();
        }
    }

} // namespace ui
