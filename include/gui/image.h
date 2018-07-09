#pragma once

#include <gui/element.h>

namespace ui {

	// TODO: allow sharing resources, maybe via std::shared_ptr<sf::Texture>?

	struct Image : ui::InlineElement {
		// construct a blank image
		Image(unsigned width = 100, unsigned height = 100, sf::Color color = sf::Color(0xFFFFFFFF));

		// load an image from a file path
		Image(const std::string& path, bool auto_size = true);

		// construct from an existing image
		Image(const sf::Image& img, bool auto_size = true);

		// get the stored image
		sf::Image& getImage();

		// apply changes made to the image so that they can be seen when rendered
		// should only be called after modifying the internal image through getImage()
		bool update();

		// reset the stored image
		bool create(unsigned width = 100, unsigned height = 100, sf::Color color = sf::Color(0xFFFFFFFF));

		// load an image from a file path
		bool loadFromFile(const std::string& path, bool auto_size = true);

		// set the opacity, from 0 (fully transparent) to 255 (fully opaque)
		void setAlpha(uint8_t alpha);

		// get the opacity, from 0 (fully transparent) to 255 (fully opaque)
		uint8_t getAlpha() const;

		// set the color that the image is multiplied by
		void setColorMod(sf::Color color);

		// get the color that the image is multiplied by
		sf::Color getColorMod() const;

		// copy from an existing image
		bool copyFrom(const sf::Image& img, bool auto_size = true);

		void onResize() override;

		void render(sf::RenderWindow& rw) const override;

	private:

		sf::Image image;
		sf::Texture texture;
		sf::Sprite sprite;
	};

} // namespace ui