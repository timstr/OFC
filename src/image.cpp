#include "gui/image.h"

ui::Image::Image(const std::string& path, bool auto_size) {
	loadFromFile(path, auto_size);
}

ui::Image::Image(const sf::Image& img, bool auto_size) {
	copyFrom(img, auto_size);
}

ui::Image::Image(const std::shared_ptr<sf::Texture>& _texture, bool auto_size) {
	setTexture(_texture, auto_size);
}

const std::shared_ptr<sf::Texture>& ui::Image::getTexture() const {
	return texture;
}

bool ui::Image::loadFromFile(const std::string& path, bool auto_size) {
	sf::Image image;
	if (!image.loadFromFile(path)) {
		return false;
	}
	return copyFrom(image, auto_size);
}

void ui::Image::setAlpha(uint8_t alpha) {
	sf::Color colormod = sprite.getColor();
	colormod.a = alpha;
	sprite.setColor(colormod);
}

uint8_t ui::Image::getAlpha() const {
	return sprite.getColor().a;
}

void ui::Image::setColorMod(sf::Color color) {
	sprite.setColor(color);
}

sf::Color ui::Image::getColorMod() const {
	return sprite.getColor();
}

bool ui::Image::copyFrom(const sf::Image& image, bool auto_size) {
	auto tex = std::make_shared<sf::Texture>();
	if (!tex->loadFromImage(image)) {
		return false;
	}
	return setTexture(tex, auto_size);
}

bool ui::Image::setTexture(const std::shared_ptr<sf::Texture>& _texture, bool auto_size) {
	texture = _texture;
	if (!texture) {
		return false;
	}
	if (auto_size) {
		auto s = texture->getSize();
		setSize({ (float)s.x, (float)s.y }, true);
	}
	sprite.setTexture(*texture);
	return true;
}

void ui::Image::onResize() {
	sprite.setScale({
		getSize().x / (float)texture->getSize().x,
		getSize().y / (float)texture->getSize().y
	});
}

void ui::Image::render(sf::RenderWindow& rw) const {
	rw.draw(sprite);
}
