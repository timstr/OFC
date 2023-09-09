#pragma once

#ifndef TIMSGUI_KEY_H
#define TIMSGUI_KEY_H

#include <SFML/Graphics.hpp>
#include <memory>

typedef sf::Vector2f vec2;

namespace ui {

	using Key = sf::Keyboard::Key;

	// Strong, i.e. owning reference to an element.
	// Storing this reference will prevent the referred-to
	// element from being destroyed. Care should be taken
	// to avoid cycles which would cause memory leaks.
	template<typename ElementType>
	using Ref = std::shared_ptr<ElementType>;

	// Weak, i.e. non-owning reference to an element.
	// Can be used to create a strong reference using the 'lock()' method.
	// Storing this reference will allow the referred-to element
	// to be destroyed
	template<typename ElementType>
	using WeakRef = std::weak_ptr<ElementType>;

} // namespace ui

#endif // TIMSGUI_KEY_H
