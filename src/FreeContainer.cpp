#include <GUI/FreeContainer.hpp>

#include <cassert>

namespace ui {

    void FreeContainer::adopt(std::unique_ptr<Element> e){
        const Element* eptr = e.get();
        Container::adopt(std::move(e));
        m_styles.try_emplace(eptr, ElementStyle{PositionStyle::None, PositionStyle::None});
    }

    void FreeContainer::adopt(PositionStyle xstyle, PositionStyle ystyle, std::unique_ptr<Element> e){
        const Element* eptr = e.get();
        Container::adopt(std::move(e));
        m_styles.try_emplace(eptr, ElementStyle{xstyle, ystyle});
    }

    std::unique_ptr<Element> FreeContainer::release(const Element* e){
        auto it = m_styles.find(e);
        if (it != m_styles.end()){
            m_styles.erase(it);
        }
        return Container::release(e);
    }

    void FreeContainer::setElementStyle(const Element* e, PositionStyle xstyle, PositionStyle ystyle){
        auto it = m_styles.find(e);
        if (it != m_styles.end()){
            it->second.x = xstyle;
            it->second.y = ystyle;
        }
    }

    vec2 FreeContainer::update(){
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

        auto maxSize = vec2{};

        for (auto& elem : children()){
            auto it = m_styles.find(elem);
            assert(it != m_styles.end());
            const auto& style = it->second;
            const auto x = compute_position(style.x, left(), width(), elem->left(), elem->width());
            const auto y = compute_position(style.y, top(), height(), elem->top(), elem->height());
            elem->setPos({std::floor(x), std::floor(y)});
            //elem->update({0.0f, 0.0f});

            const auto isContraining = [](PositionStyle ps){
                return ps == PositionStyle::InsideBegin
                    || ps == PositionStyle::InsideEnd
                    || ps == PositionStyle::Center;
            };

            const auto req = getRequiredSize(elem);
            if (isContraining(style.x)){
                maxSize.x = std::max(maxSize.x, req.x);
            }
            if (isContraining(style.y)){
                maxSize.y = std::max(maxSize.y, req.y);
            }
        }

        if (maxSize.x > width()){
            setWidth(maxSize.x);
        }
        if (maxSize.y > height()){
            setHeight(maxSize.y);
        }

        return maxSize;
    }

    void FreeContainer::onRemoveChild(const Element* e){
        auto it = m_styles.find(e);
        assert(it != m_styles.end());
        m_styles.erase(it);
    }

} // namespace ui
