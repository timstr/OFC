#pragma once

#ifndef TIMSGUI_PI_H
#define TIMSGUI_PI_H

namespace ui {

	template<typename T>
	constexpr T pi() {
		return static_cast<T>(3.141592653589793238462643383279502884L);
	}

} // namespace ui

#endif // TIMSGUI_PI_H