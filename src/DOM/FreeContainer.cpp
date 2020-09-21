#include <OFC/DOM/FreeContainer.hpp>

#include <cassert>
#include <cmath>

namespace ofc::ui::dom {

    void FreeContainer::adopt(std::unique_ptr<Element> e){
        assert(e);
        const Element* eptr = e.get();
        Container::adopt(std::move(e));
        m_styles.try_emplace(eptr, ElementStyle{PositionStyle::None, PositionStyle::None});
    }

    void FreeContainer::adopt(PositionStyle xstyle, PositionStyle ystyle, std::unique_ptr<Element> e){
        assert(e);
        const Element* eptr = e.get();
        Container::adopt(std::move(e));
        m_styles.try_emplace(eptr, ElementStyle{xstyle, ystyle});
    }

    void FreeContainer::setElementStyle(const Element* e, PositionStyle xstyle, PositionStyle ystyle){
        auto it = m_styles.find(e);
        if (it == m_styles.end()){
            throw std::runtime_error("No such element");
        }

        if (it->second.x != xstyle || it->second.y != ystyle){
            it->second.x = xstyle;
            it->second.y = ystyle;
            requireUpdate();
        }
    }

    vec2 FreeContainer::update(){
        const auto compute_position = [](PositionStyle style, float size, float epos, float esize){
            switch (style){
            case PositionStyle::OutsideBegin:
                return -esize;
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

        const auto doUpdate = [&](){
            for (auto& elem : children()){
                auto it = m_styles.find(elem);
                assert(it != m_styles.end());
                const auto& style = it->second;
                const auto x = compute_position(style.x, width(), elem->left(), elem->width());
                const auto y = compute_position(style.y, height(), elem->top(), elem->height());
                elem->setPos({std::floor(x), std::floor(y)});
                //elem->update({0.0f, 0.0f});

                const auto isContraining = [](PositionStyle ps){
                    return ps == PositionStyle::InsideBegin
                        || ps == PositionStyle::InsideEnd
                        || ps == PositionStyle::Center;
                };

                const auto req = getRequiredSize(elem);
                if (isContraining(style.x) && isContraining(style.y)){
                    maxSize.x = std::max(maxSize.x, req.x);
                    maxSize.y = std::max(maxSize.y, req.y);
                }
            }

            auto sizeChanged = false;
            if (maxSize.x > width()){
                setWidth(maxSize.x);
                sizeChanged = true;
            }
            if (maxSize.y > height()){
                setHeight(maxSize.y);
                sizeChanged = true;
            }
            return sizeChanged;
        };

        if (doUpdate()){
            doUpdate();
        }

        return maxSize;
    }

    void FreeContainer::onRemoveChild(const Element* e){
        auto it = m_styles.find(e);
        assert(it != m_styles.end());
        m_styles.erase(it);
    }

} // namespace ofc::ui::dom
