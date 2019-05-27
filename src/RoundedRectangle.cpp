#include <GUI/RoundedRectangle.hpp>

namespace ui {

	RoundedRectangle::RoundedRectangle(sf::Vector2f _size, float _radius)
		: m_radius(_radius),
		m_size(_size) {

	}

	void RoundedRectangle::setRadius(float _radius) {
		m_radius = std::max(_radius, 0.0f);
		updatePoints();
	}

	float RoundedRectangle::getRadius() const {
		return m_radius;
	}

	void RoundedRectangle::setSize(sf::Vector2f _size) {
		m_size = { std::max(_size.x, 0.0f), std::max(_size.y, 0.0f) };
		updatePoints();
	}

	sf::Vector2f RoundedRectangle::getSize() const {
		return m_size;
	}
	
	void RoundedRectangle::updatePoints() {
		std::vector<sf::Vector2f> points;

		const float rad = std::min(m_radius, std::min(m_size.x * 0.5f, m_size.y * 0.5f));
		if (rad == 0.0f) {
			points.push_back({ 0.0f, 0.0f });
			points.push_back({ m_size.x, 0.0f });
			points.push_back({ m_size.x, m_size.y });
			points.push_back({ 0.0f, m_size.y });
		} else {
			const float pi = 3.141592654f;
			float inc = pi / rad;

			// top left corner
			for (float a = 0.0; a < pi * 0.5f; a += inc) {
				sf::Vector2f pos {
					rad * (1.0f - cos(a)),
					rad * (1.0f - sin(a))
				};
				points.push_back(pos);
			}
			points.push_back({ rad, 0.0f });

			// top right corner
			for (float a = 0.0f; a < pi * 0.5f; a += inc) {
				sf::Vector2f pos {
					m_size.x + rad * (-1.0f + sin(a)),
					rad * (1.0f - cos(a))
				};
				points.push_back(pos);
			}
			points.push_back({ m_size.x, rad });

			// bottom right corner
			for (float a = 0.0f; a < pi * 0.5f; a += inc) {
				sf::Vector2f pos {
					m_size.x + rad * (-1.0f + cos(a)),
					m_size.y + rad * (-1.0f + sin(a))
				};
				points.push_back(pos);
			}
			points.push_back({ m_size.x - rad, m_size.y });

			// bottom left corner
			for (float a = 0.0f; a < pi * 0.5f; a += inc) {
				sf::Vector2f pos {
					rad * (1.0f - sin(a)),
					m_size.y + rad * (-1.0f + cos(a))
				};
				points.push_back(pos);
			}
			points.push_back({ 0.0f, m_size.y - rad });
		}

		setPointCount(points.size());
		for (std::size_t i = 0; i < points.size(); ++i) {
			setPoint(i, points[i]);
		}
	}

} // namespace ui