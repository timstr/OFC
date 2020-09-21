#pragma once

#include <cstdint>

namespace ofc::ui {

    enum class HorizontalDirection : std::uint8_t {
        LeftToRight,
        RightToLeft
    };

    enum class VerticalDirection : std::uint8_t {
        TopToBottom,
        BottomToTop
    };

    inline constexpr auto LeftToRight = HorizontalDirection::LeftToRight;
    inline constexpr auto RightToLeft = HorizontalDirection::RightToLeft;

    inline constexpr auto TopToBottom = VerticalDirection::TopToBottom;
    inline constexpr auto BottomToTop = VerticalDirection::BottomToTop;

} // namespace ofc::ui
