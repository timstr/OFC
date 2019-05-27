#pragma once

#ifndef TIMSGUI_ROUNDEDRECTANGLE_H
#define TIMSGUI_ROUNDEDRECTANGLE_H

#include <SFML/Graphics.hpp>

namespace ui {

	class RoundedRectangle : public sf::ConvexShape {
    public:
		RoundedRectangle(sf::Vector2f _size = {}, float _radius = 0.0f);

		void setRadius(float _radius);

		float getRadius() const;

		void setSize(sf::Vector2f _size);

		sf::Vector2f getSize() const;

	private:
		float m_radius;
		sf::Vector2f m_size;

		void updatePoints();
	};

} // namespace ui

#endif // TIMSGUI_ROUNDEDRECTANGLE_H