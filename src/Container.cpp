#include <GUI/Container.hpp>

#include <GUI/Window.hpp>

#include <algorithm>
#include <cassert>

namespace ui {

    // TODO: put all the search code into a single helper function

    Container::Container()
        : m_parentWindow(nullptr) {
    }

    Container::~Container(){
        if (auto win = getParentWindow()){
            win->onRemoveElement(this);
        }
    }
    
    void Container::adopt(std::unique_ptr<Element> e){
        e->m_parent = this;
        auto eptr = e.get();
        m_children.push_back({std::move(e), {}, {}, {}});
        requireDeepUpdate();
    }

    std::unique_ptr<Element> Container::release(const Element* e){
        auto it = std::find_if(
            m_children.begin(),
            m_children.end(),
            [e](const ChildData& cd){
                return cd.child.get() == e;
            }
        );
        if (it == m_children.end()){
            throw std::runtime_error("Attempted to remove a nonexistent child window");
        }
        if (auto win = getParentWindow()){
            win->onRemoveElement(it->child.get());
        }

        std::function<void(Element*)> callOnRemove = [&](Element* elem){
            elem->onRemove();
            if (const auto& cont = elem->toContainer()){
                for (const auto& ch : cont->children()){
                    callOnRemove(ch);
                }
            }
        };
        callOnRemove(it->child.get());


        onRemoveChild(it->child.get());
        std::unique_ptr<Element> ret = std::move(it->child);
        assert(ret->m_parent == this);
        ret->m_parent = nullptr;
        m_children.erase(it);
        requireUpdate();
        return ret;
    }

    void Container::onRemoveChild(const Element*){

    }

    std::vector<Element*> Container::children(){
        std::vector<Element*> ret;
        std::transform(
            m_children.begin(),
            m_children.end(),
            std::back_inserter(ret),
            [](const ChildData& cd){
                return cd.child.get();
            }
        );
        return ret;
    }

    std::vector<const Element*> Container::children() const {
        std::vector<const Element*> ret;
        std::transform(
            m_children.begin(),
            m_children.end(),
            std::back_inserter(ret),
            [](const ChildData& cd){
                return cd.child.get();
            }
        );
        return ret;
    }

    void Container::setAvailableSize(const Element* child, vec2 size){
        auto it = std::find_if(
            m_children.begin(),
            m_children.end(),
            [&](const ChildData& cd){
                return cd.child.get() == child;
            }
        );
        assert(it != m_children.end());
        if (!it->availableSize.has_value() ||
            (std::abs(it->availableSize->x - size.x) > 1e-6 || std::abs(it->availableSize->y - size.y) > 1e-6)){
            it->child->requireUpdate();
        }
        it->previousSize.reset();
        it->availableSize = size;
    }

    void Container::unsetAvailableSize(const Element* child){
        auto it = std::find_if(
            m_children.begin(),
            m_children.end(),
            [&](const ChildData& cd){
                return cd.child.get() == child;
            }
        );
        assert(it != m_children.end());
        if (it->availableSize.has_value()){
            it->child->requireUpdate();
        }
        it->previousSize.reset();
        it->availableSize.reset();
    }

    std::optional<vec2> Container::getAvailableSize(const Element* child) const {
        auto it = std::find_if(
            m_children.begin(),
            m_children.end(),
            [&](const ChildData& cd){
                return cd.child.get() == child;
            }
        );
        assert(it != m_children.end());
        return it->availableSize;
    }

    vec2 Container::getRequiredSize(const Element* child) const {
        auto it = std::find_if(
            m_children.begin(),
            m_children.end(),
            [&](const ChildData& cd){
                return cd.child.get() == child;
            }
        );
        assert(it != m_children.end());
        if (!it->requiredSize){
            it->child->requireUpdate();
        }
        it->child->forceUpdate();
        assert(it->requiredSize);
        return *it->requiredSize;
    }

    void Container::updatePreviousSizes(){
        for (auto& cd : m_children){
            cd.previousSize = cd.child->m_size;
        }
    }

    std::optional<vec2> Container::getPreviousSize(const Element* child) const {
        auto it = std::find_if(
            m_children.begin(),
            m_children.end(),
            [&](const ChildData& cd){
                return cd.child.get() == child;
            }
        );
        assert(it != m_children.end());
        return it->previousSize;
    }

    void Container::updatePositions(){
        for (auto& cd : m_children){
            const auto pos = cd.child->m_position;
            if (!cd.previousPos || (std::abs(cd.previousPos->x - pos.x) > 1e-6f || std::abs(cd.previousSize->y - pos.y) > 1e-6f)){
                cd.previousPos = pos;
                cd.child->onMove();
            }
        }
    }

    void Container::setRequiredSize(const Element* child, vec2 size){
        auto it = std::find_if(
            m_children.begin(),
            m_children.end(),
            [&](const ChildData& cd){
                return cd.child.get() == child;
            }
        );
        assert(it != m_children.end());
        it->requiredSize = size;
    }

    void Container::render(sf::RenderWindow& rw){
        const auto old_view = rw.getView();
        // TODO: handle clipping here
        for (const auto& cd : m_children){
            auto child_view = old_view;
            auto pos = cd.child->pos();
            child_view.move(-pos);
            rw.setView(child_view);
            cd.child->render(rw);
        }
        rw.setView(old_view);
    }

    Container* Container::toContainer(){
        return this;
    }

    Element* Container::findElementAt(vec2 p){
        for (auto it = m_children.rbegin(), end = m_children.rend(); it != end; ++it){
            auto& c = it->child;
            if (auto e = c->findElementAt(p - pos())){
                return e;
            }
        }
        return hit(p) ? this : nullptr;
    }

    Window* Container::getWindow() const {
        if (m_parentWindow){
            return m_parentWindow;
        }
        if (auto p = getParentContainer()){
            return p->getWindow();
        }
        return nullptr;
    }
    

} // namespace ui
