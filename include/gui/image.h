#pragma once

#include <gui/element.h>

namespace ui {

	struct Image : ui::InlineElement {
		// load an image from a file path
		// using the same path multiple times will lead to many texture copies and wasted memory
		// in this case, consider using a shared texture
		Image(const std::string& path, bool auto_size = true);

		// copy from an existing image
		// if used multiple times, this will lead to many texture copies and wasted memory
		// in this case, consider using a shared texture
		Image(const sf::Image& img, bool auto_size = true);

		// use an existing shared texture
		Image(const Ref<sf::Texture>& _texture, bool autosize = true);

		// get the shared texture
		const Ref<sf::Texture>& getTexture() const;

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

		// use a shared texture
		bool setTexture(const Ref<sf::Texture>& _texture, bool auto_size = true);

	private:

		void onResize() override;

		void render(sf::RenderWindow& rw) override;

		Ref<sf::Texture> texture;
		sf::Sprite sprite;
	};

} // namespace ui