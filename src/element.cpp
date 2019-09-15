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
        m_fill_x(false),
        m_fill_y(false),
        m_needs_update(false),
        m_parent(nullptr) {
        
    }
    Element::~Element(){
        if (Window* win = getWindow()){
            win->onRemoveElement(this);
        }
    }
    float Element::left() const {
        return m_position.x;
    }
    float Element::top() const {
        return m_position.y;
    }
    vec2 Element::pos() const {
        return m_position;
    }
    void Element::setLeft(float v){
        v = std::abs(v);
        if (different(v, m_position.x)){
            m_position.x = v;
            if (m_parent){
                m_parent->require_update();
            }
        }
    }
    void Element::setTop(float v){
        v = std::abs(v);
        if (different(v, m_position.y)){
            m_position.y = v;
            if (m_parent){
                m_parent->require_update();
            }
        }
    }
    void Element::setPos(vec2 v){
        v.x = std::abs(v.x);
        v.y = std::abs(v.y);
        if (different(v, m_position)){
            m_position = v;
            if (m_parent){
                m_parent->require_update();
            }
        }
    }
    vec2 Element::rootPos() const {
        return m_position + (m_parent ? m_parent->rootPos() : vec2{});
    }
    vec2 Element::localMousePos() const {
        auto win = getParentWindow();
        assert(win);
        return win->getMousePosition() - rootPos();
    }
    void Element::onMove(){

    }
    float Element::width() const {
        return m_size.x;
    }
    float Element::height() const {
        return m_size.y;
    }
    vec2 Element::size() const {
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
            require_update();
            if (m_parent){
                m_parent->require_update();
            }
            onResize();
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
            require_update();
            if (m_parent){
                m_parent->require_update();
            }
            onResize();
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
    void Element::setHorizontalFill(bool v){
        m_fill_x = v;
    }
    void Element::setVerticalFill(bool v){
        m_fill_y = v;
    }
    bool Element::horizontalFill() const {
        return m_fill_x;
    }
    bool Element::verticalFill() const {
        return m_fill_y;
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
    void Element::startTransition(float duration, std::function<void(float)> fn, std::function<void()> on_complete){
        if (auto win = getParentWindow()){
            win->addTransition(
                this,
                duration,
                std::move(fn),
                std::move(on_complete)
            );
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

    void Element::require_update(){
        m_needs_update = true;
    }

    void Element::update(vec2 max_size){
        if (m_fill_x){
            setWidth(std::clamp(max_size.x, m_minsize.x, m_maxsize.x));
        }
        if (m_fill_y){
            setHeight(std::clamp(max_size.y, m_minsize.y, m_maxsize.y));
        }
        if (m_needs_update){
            updateContents();
        }
        m_needs_update = false;
    }

    void Element::updateContents(){
        applyTransitions();
    }

} // namespace ui
