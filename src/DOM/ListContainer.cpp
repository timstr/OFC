#include <OFC/DOM/ListContainer.hpp>

namespace ofc::ui::dom {

    VerticalList::VerticalList(VerticalDirection direction, bool expand)
        : ListContainer(direction == TopToBottom, expand) {

    }

    void VerticalList::setDirection(VerticalDirection dir) {
        ListContainer::setDirection(dir == TopToBottom);
    }

    HorizontalList::HorizontalList(HorizontalDirection direction, bool expand)
        : ListContainer(direction == LeftToRight, expand) {

    }

    void HorizontalList::setDirection(HorizontalDirection dir) {
        ListContainer::setDirection(dir == LeftToRight);
    }

} // namespace ofc::ui::dom
