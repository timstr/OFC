#pragma once

#include <GUI/DOM/Element.hpp>
#include <GUI/Util/Color.hpp>

#include <SFML/Graphics.hpp>
#include <memory>

namespace ui::dom {

    // TODO: enable spritesheets by allowing a clipping rect to be passed along with shared pointer to texture

    class Image : public Element {
    public:
        // load an image from a file path
		// using the same path multiple times will lead to many texture copies and wasted memory
		// in this case, consider using a shared texture
		Image(const std::string& path, bool autoSize = true);

		// copy from an existing image
		// if used multiple times, this will lead to many texture copies and wasted memory
		// in this case, consider using a shared texture
		Image(const sf::Image& img, bool autoSize = true);

		// use an existing shared texture
		Image(std::shared_ptr<sf::Texture> texture, bool autoSize = true);

        // get the texture by reference
        sf::Texture& getTexture();
        const sf::Texture& getTexture() const;

		// get the texture by shared pointer
		std::shared_ptr<sf::Texture> getTexturePtr();
		std::shared_ptr<const sf::Texture> getTexturePtr() const;

		// load an image from a file path
		bool loadFromFile(const std::string& path, bool autoSize = true);

		// copy from an existing image
		bool copyFromImage(const sf::Image& img, bool autoSize = true);

		// use a shared texture
		bool setTexture(std::shared_ptr<sf::Texture> texture, bool autoSize = true);

		// set the opacity, from 0 (fully transparent) to 255 (fully opaque)
		void setAlpha(uint8_t alpha);

		// get the opacity, from 0 (fully transparent) to 255 (fully opaque)
		uint8_t alpha() const;

		// set the color that the image is multiplied by
		void setColorMod(Color color);

		// get the color that the image is multiplied by
		Color colorMod() const;

	private:

		void onResize() override;

		void render(sf::RenderWindow& rw) override;

		std::shared_ptr<sf::Texture> m_texture;
		sf::Sprite m_sprite;
	};

} // namespace ui::dom
