#pragma once

#ifndef TIMSGUI_ROUNDEDRECTANGLE_H
#define TIMSGUI_ROUNDEDRECTANGLE_H

#include "SFML/Graphics.hpp"

namespace ui {

	struct RoundedRectangle : sf::ConvexShape {
		RoundedRectangle(sf::Vector2f _size = {}, float _radius = 0.0f);

		void setRadius(float _radius);

		float getRadius() const;

		void setSize(sf::Vector2f _size);

		sf::Vector2f getSize() const;

	private:
		float radius;
		sf::Vector2f size;


		void updatePoints();
	};

} // namespace ui

#endif // TIMSGUI_ROUNDEDRECTANGLE_H