#pragma once

#include <cstdint>

namespace ui {

    enum HorizontalDirection : std::uint8_t {
        RightToLeft,
        LeftToRight
    };

    enum VerticalDirection : std::uint8_t {
        TopToBottom,
        BottomToTop
    };

} // namespace ui
