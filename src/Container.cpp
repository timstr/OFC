#include <GUI/Container.hpp>

#include <GUI/Window.hpp>

#include <algorithm>
#include <cassert>

namespace ui {

    // TODO: put all the search code into a single helper function

    Container::Container()
        : m_parentWindow(nullptr)
        , m_clipping(false)
        , m_shrink(false) {

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

    void Container::clear(){
        while (m_children.size() > 0){
            release(m_children.back().child.get());
        }
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

    bool Container::clipping() const {
        return m_clipping;
    }

    void Container::setClipping(bool enabled){
        m_clipping = enabled;
    }

    bool Container::shrink() const {
        return m_shrink;
    }

    void Container::setShrink(bool enable){
        if (m_shrink != enable){
            m_shrink = enable;
            if (m_shrink){
                requireDeepUpdate();
            }
        }
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
        //it->previousSize.reset();
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
        //it->previousSize.reset();
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
        // TODO: an assertion here to make sure requiredSize was not empty was causing problems
        // This is a cheap workaround, but this should be investigated
        if (it->requiredSize){
            return *it->requiredSize;
        }
        return {};
    }

    void Container::updatePreviousSizes(const Element* e){
        for (auto& cd : m_children){
            if (e && e != cd.child.get()){
                continue;
            }
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

    void Container::updatePositions(const Element* e){
        for (auto& cd : m_children){
            if (e && e != cd.child.get()){
                continue;
            }
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
        const auto oldView = rw.getView();

        auto clippedView = oldView;
        if (m_clipping){

            const auto oldVP = oldView.getViewport();
            
            const auto rp = rootPos();
            const auto newTopLeft = vec2{
                rp.x / static_cast<float>(rw.getSize().x),
                rp.y / static_cast<float>(rw.getSize().y)
            };
            const auto newBottomRight = vec2{
                (rp.x + width()) / static_cast<float>(rw.getSize().x),
                (rp.y + height()) / static_cast<float>(rw.getSize().y)
            };
            
            const auto effTopLeft = vec2{
                std::max(oldVP.left, newTopLeft.x),
                std::max(oldVP.top, newTopLeft.y)
            };
            const auto effBottomRight = vec2{
                std::min(oldVP.left + oldVP.width, newBottomRight.x),
                std::min(oldVP.top + oldVP.height, newBottomRight.y)
            };

            if (effTopLeft.x >= effBottomRight.x || effTopLeft.y >= effBottomRight.y){
                return;
            }

            clippedView.setViewport(sf::FloatRect{
                effTopLeft,
                effBottomRight - effTopLeft
            });

            clippedView.setCenter(size() * 0.5f);
            clippedView.setSize(size());
        }
        
        for (const auto& cd : m_children){
            if (!cd.child->visible()){
                continue;
            }
            auto childView = clippedView;
            auto pos = cd.child->pos();
            childView.move(-pos);
            rw.setView(childView);
            cd.child->render(rw);
        }

        rw.setView(oldView);
    }

    Container* Container::toContainer(){
        return this;
    }

    Element* Container::findElementAt(vec2 p, const Element* exclude){
        if (this == exclude){
            return nullptr;
        }
        const bool hitThis = hit(p);
        if (m_clipping && !hitThis){
            return nullptr;
        }
        for (auto it = m_children.rbegin(), end = m_children.rend(); it != end; ++it){
            auto& c = it->child;
            if (auto e = c->findElementAt(p - pos(), exclude)){
                return e;
            }
        }
        return hitThis ? this : nullptr;
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
