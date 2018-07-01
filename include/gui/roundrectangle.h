#pragma once

#include "SFML/Graphics.hpp"

namespace ui {

	struct RoundedRectangle : sf::ConvexShape {
		RoundedRectangle(sf::Vector2f _size = {}, float _radius = 0.0f) : radius(_radius), size(_size) {

		}

		void setRadius(float _radius){
			radius = std::max(_radius, 0.0f);
			updatePoints();
		}

		float getRadius() const {
			return radius;
		}

		void setSize(sf::Vector2f _size){
			size = {std::max(_size.x, 0.0f), std::max(_size.y, 0.0f)};
			updatePoints();
		}

		sf::Vector2f getSize() const {
			return size;
		}

	private:
		float radius;
		sf::Vector2f size;
		

		void updatePoints(){
			std::vector<sf::Vector2f> points;

			const float rad = std::min(radius, std::min(size.x * 0.5f, size.y * 0.5f));
			if (rad == 0.0f){
				points.push_back({0.0f, 0.0f});
				points.push_back({size.x, 0.0f});
				points.push_back({size.x, size.y});
				points.push_back({0.0f, size.y});
			} else {
				const float pi = 3.141592654f;
				float inc = pi / rad;

				// top left corner
				for (float a = 0.0; a < pi * 0.5f; a += inc){
					sf::Vector2f pos {
						rad * (1.0f - cos(a)),
						rad * (1.0f - sin(a))
					};
					points.push_back(pos);
				}
				points.push_back({rad, 0.0f});

				// top right corner
				for (float a = 0.0f; a < pi * 0.5f; a += inc){
					sf::Vector2f pos {
						size.x + rad * (-1.0f + sin(a)),
						rad * (1.0f - cos(a))
					};
					points.push_back(pos);
				}
				points.push_back({size.x, rad});

				// bottom right corner
				for (float a = 0.0f; a < pi * 0.5f; a += inc){
					sf::Vector2f pos {
						size.x + rad * (-1.0f + cos(a)),
						size.y + rad * (-1.0f + sin(a))
					};
					points.push_back(pos);
				}
				points.push_back({size.x - rad, size.y});

				// bottom left corner
				for (float a = 0.0f; a < pi * 0.5f; a += inc){
					sf::Vector2f pos {
						rad * (1.0f - sin(a)),
						size.y + rad * (-1.0f + cos(a))
					};
					points.push_back(pos);
				}
				points.push_back({0.0f, size.y - rad});
			}

			setPointCount(points.size());
			for (std::size_t i = 0; i < points.size(); ++i){
				setPoint(i, points[i]);
			}
		}
	};

} // namespace ui