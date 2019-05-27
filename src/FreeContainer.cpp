#include <GUI/FreeContainer.hpp>

#include <cassert>

namespace ui {

    void FreeContainer::adopt(std::unique_ptr<Element> e){
        const Element* eptr = e.get();
        Container::adopt(std::move(e));
        m_styles.try_emplace(eptr, std::pair{PositionStyle::None, PositionStyle::None});
    }

    void FreeContainer::adopt(std::unique_ptr<Element> e, PositionStyle xstyle, PositionStyle ystyle){
        const Element* eptr = e.get();
        Container::adopt(std::move(e));
        m_styles.try_emplace(eptr, std::pair{xstyle, ystyle});
    }

    std::unique_ptr<Element> FreeContainer::release(const Element* e){
        auto it = m_styles.find(e);
        if (it != m_styles.end()){
            m_styles.erase(it);
        }
        return Container::release(e);
    }

    void FreeContainer::updateContents(){
        const auto compute_position = [](PositionStyle style, float pos, float size, float epos, float esize){
            switch (style){
            case PositionStyle::OutsideBegin:
                return -epos;
            case PositionStyle::InsideBegin:
                return 0.0f;
            case PositionStyle::Center:
                return size * 0.5f - esize * 0.5f;
            case PositionStyle::InsideEnd:
                return size - esize;
            case PositionStyle::OutsideEnd:
                return size;
            default:
                return epos;
            }
        };

        for (auto& elem : children()){
            auto it = m_styles.find(elem.get());
            assert(it != m_styles.end());
            const auto& [xstyle, ystyle] = it->second;
            float x = compute_position(xstyle, left(), width(), elem->left(), elem->width());
            float y = compute_position(ystyle, top(), height(), elem->top(), elem->height());
            elem->setPos({x, y});
            elem->update({0.0f, 0.0f});
        }
    }

} // namespace ui
