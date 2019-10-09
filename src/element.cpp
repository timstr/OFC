#include <GUI/Element.hpp> 
#include <GUI/Container.hpp>
#include <GUI/Context.hpp>
#include <GUI/Window.hpp>

#include <cassert>

namespace ui {
    
    namespace {
        bool different(float a, float b){
            return abs(a - b) > 1e-6f;
        }

        bool different(const vec2& a, const vec2& b){
            return (abs(a.x - b.x) > 1e-6f) || (abs(a.y - b.y) > 1e-6f);
        }

        const float really_big = 1e6f;
    };

    Element::Element() :
        m_position({0.0f, 0.0f}),
        m_size({100.0f, 100.0f}),
        m_minsize({0.0f, 0.0f}),
        m_maxsize({really_big, really_big}),
        m_needs_update(false),
        m_isUpdating(false),
        m_parent(nullptr) {
        
    }
    Element::~Element(){
        if (Window* win = getWindow()){
            win->onRemoveElement(this);
        }
    }
    float Element::left(){
        forceUpdate();
        return m_position.x;
    }
    float Element::top(){
        forceUpdate();
        return m_position.y;
    }
    vec2 Element::pos(){
        forceUpdate();
        return m_position;
    }
    void Element::setLeft(float v){
        if (different(v, m_position.x)){
            m_position.x = v;
            if (m_parent){
                m_parent->requireUpdate();
            }
        }
    }
    void Element::setTop(float v){
        if (different(v, m_position.y)){
            m_position.y = v;
            if (m_parent){
                m_parent->requireUpdate();
            }
        }
    }
    void Element::setPos(vec2 v){
        if (different(v, m_position)){
            m_position = v;
            if (m_parent){
                m_parent->requireUpdate();
            }
        }
    }
    vec2 Element::rootPos(){
        return pos() + (m_parent ? m_parent->rootPos() : vec2{});
    }
    vec2 Element::localMousePos(){
        auto win = getParentWindow();
        assert(win);
        return win->getMousePosition() - rootPos();
    }
    void Element::onMove(){

    }
    float Element::width(){
        forceUpdate();
        return m_size.x;
    }
    float Element::height(){
        forceUpdate();
        return m_size.y;
    }
    vec2 Element::size(){
        forceUpdate();
        return m_size;
    }
    void Element::setWidth(float v, bool force){
        v = std::abs(v);
        if (force){
            m_minsize.x = m_maxsize.x = v;
        } else {
            v = std::clamp(v, m_minsize.x, m_maxsize.x);
        }
        if (different(v, m_size.x)){
            m_size.x = v;
            requireUpdate();
        }
    }
    void Element::setMinWidth(float v){
        v = std::abs(v);
        m_minsize.x = v;
        if (m_maxsize.x < v){
            m_maxsize.x = v;
        }
        if (m_size.x < v){
            setWidth(v);
        }
    }
    void Element::setMaxWidth(float v){
        v = std::abs(v);
        m_maxsize.x = v;
        if (m_minsize.x > v){
            m_minsize.x = v;
        }
        if (m_size.x > v){
            setWidth(v);
        }
    }
    void Element::setHeight(float v, bool force){
        v = std::abs(v);
        if (force){
            m_minsize.y = m_maxsize.y = v;
        } else {
            v = std::clamp(v, m_minsize.y, m_maxsize.y);
        }
        if (different(v, m_size.y)){
            m_size.y = v;
            requireUpdate();
        }
    }
    void Element::setMinHeight(float v){
        v = std::abs(v);
        m_minsize.y = v;
        if (m_maxsize.y < v){
            m_maxsize.y = v;
        }
        if (m_size.y < v){
            setHeight(v);
        }
    }
    void Element::setMaxHeight(float v){
        v = std::abs(v);
        m_maxsize.y = v;
        if (m_minsize.y > v){
            m_minsize.y = v;
        }
        if (m_size.y > v){
            setHeight(v);
        }
    }
    void Element::setSize(vec2 v, bool force){
        setWidth(v.x, force);
        setHeight(v.y, force);
    }
    void Element::setMinSize(vec2 v){
        setMinWidth(v.x);
        setMinHeight(v.y);
    }
    void Element::setMaxSize(vec2 v){
        setMaxWidth(v.x);
        setMaxHeight(v.y);
    }
    void Element::onResize(){

    }
    bool Element::hit(vec2 p) const {
        return
            (p.x >= m_position.x) &&
            (p.x < m_position.x + m_size.x) &&
            (p.y >= m_position.y) &&
            (p.y < m_position.y + m_size.y);
    }
    Element* Element::findElementAt(vec2 p){
        return hit(p) ? this : nullptr;
    }
    void Element::startTransition(double duration, std::function<void(double)> fn, std::function<void()> on_complete){
        if (auto win = getParentWindow()){
            win->addTransition(
                this,
                duration,
                std::move(fn),
                std::move(on_complete)
            );
        }
    }

    void Element::clearTransitions(){
        if (auto win = getParentWindow()){
            win->removeTransitions(this);
        }
    }

    std::unique_ptr<Element> Element::orphan(){
        if (!m_parent){
            throw std::runtime_error("Attempted to orphan an element without a parent");
        }
        return m_parent->release(this);
    }
    void Element::close(){
        orphan();
    }
    Container* Element::getParentContainer(){
        return m_parent;
    }
    const Container* Element::getParentContainer() const {
        return m_parent;
    }
    Window* Element::getParentWindow() const {
        return getWindow();
    }
    Container* Element::toContainer(){
        return nullptr;
    }
    Control* Element::toControl(){
        return nullptr;
    }
    Draggable* Element::toDraggable(){
        return nullptr;
    }
    Text* Element::toText(){
        return nullptr;
    }
    TextEntry* Element::toTextEntry(){
        return nullptr;
    }
    Window* Element::getWindow() const {
        if (m_parent){
            return m_parent->getWindow();
        }
        return nullptr;
    }

    Control* Element::getParentControl(){
        if (m_parent){
            if (auto control = m_parent->toControl()){
                return control;
            }
            return m_parent->getParentControl();
        }
        return nullptr;
    }

    const Control* Element::getParentControl() const {
        if (m_parent){
            if (auto control = m_parent->toControl()){
                return control;
            }
            return m_parent->getParentControl();
        }
        return nullptr;
    }

    void Element::requireUpdate(){
        if (!m_needs_update && !m_isUpdating){
            if (auto win = getParentWindow()){
                win->enqueueForUpdate(this);
                m_needs_update = true;
            }
        }
    }

    vec2 Element::update(){
        // Nothing to do by default
        return m_size;
    }

    void Element::forceUpdate(){
        if (m_needs_update && !m_isUpdating){
            if (auto win = getParentWindow()){
                // NOTE: this may cause the parent to be updated as well,
                // if the element's size changes
                win->updateOneElement(this);
                assert(!m_needs_update);
            }
        }
    }

    void Element::requireDeepUpdate(){
        auto win = getParentWindow();
        if (!win){
            return;
        }
        std::function<void(Element*)> fn = [&](Element* e){
            e->requireUpdate();
            if (auto cont = e->toContainer()){
                for (auto c : cont->children()){
                    fn(c);
                }
            }
        };
        fn(this);
    }

} // namespace ui
