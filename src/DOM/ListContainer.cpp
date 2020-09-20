#include <GUI/DOM/ListContainer.hpp>

namespace ui::dom {

    VerticalList::VerticalList(VerticalDirection direction)
        : ListContainer(direction == TopToBottom) {

    }

    HorizontalList::HorizontalList(HorizontalDirection direction)
        : ListContainer(direction == LeftToRight) {

    }

} // namespace ui::dom
