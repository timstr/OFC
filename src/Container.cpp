#include <GUI/Container.hpp>

#include <algorithm>
#include <cassert>

namespace ui {

    Container::Container()
        : m_parentWindow(nullptr) {
    }
    
    void Container::adopt(std::unique_ptr<Element> e){
        e->m_parent = this;
        auto eptr = e.get();
        m_children.push_back({std::move(e), {}, {0.0f, 0.0f}});
        eptr->requireDeepUpdate();
        requireUpdate();
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
        std::unique_ptr<Element> ret = std::move(it->child);
        m_children.erase(it);
        requireUpdate();
        return ret;
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

    void Container::updatePreviousSizes(){
        for (auto& cd : m_children){
            cd.previousSize = cd.child->m_size;
        }
    }

    vec2 Container::getPreviousSize(const Element* child) const {
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
