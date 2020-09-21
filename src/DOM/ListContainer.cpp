#include <OFC/DOM/ListContainer.hpp>

namespace ofc::ui::dom {

    VerticalList::VerticalList(VerticalDirection direction)
        : ListContainer(direction == TopToBottom) {

    }

    HorizontalList::HorizontalList(HorizontalDirection direction)
        : ListContainer(direction == LeftToRight) {

    }

} // namespace ofc::ui::dom
