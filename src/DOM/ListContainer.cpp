#include <OFC/DOM/ListContainer.hpp>

namespace ofc::ui::dom {

    VerticalList::VerticalList(VerticalDirection direction, bool expand)
        : ListContainer(direction == TopToBottom, expand) {

    }

    HorizontalList::HorizontalList(HorizontalDirection direction, bool expand)
        : ListContainer(direction == LeftToRight, expand) {

    }

} // namespace ofc::ui::dom
