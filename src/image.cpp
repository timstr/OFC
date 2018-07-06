#include "gui/image.h"

ui::Image::Image(unsigned width, unsigned height, sf::Color color) {
	create(width, height, color);
}

ui::Image::Image(const std::string& path, bool auto_size) {
	loadFromFile(path, auto_size);
}

ui::Image::Image(const sf::Image& img, bool auto_size) {
	copyFrom(img, auto_size);
}

sf::Image& ui::Image::getImage() {
	return image;
}

bool ui::Image::update() {
	bool loaded = texture.loadFromImage(image);
	sprite.setTexture(texture, true);
	return loaded;
}

bool ui::Image::create(unsigned width, unsigned height, sf::Color color) {
	setSize({ (float)width, (float)height }, true);
	image.create(width, height, color);
	return update();
}

bool ui::Image::loadFromFile(const std::string& path, bool auto_size) {
	if (!image.loadFromFile(path)) {
		return false;
	}
	if (auto_size) {
		auto imgsize = image.getSize();
		setSize({ (float)imgsize.x, (float)imgsize.y }, true);
	}
	return update();
}

bool ui::Image::copyFrom(const sf::Image& img, bool auto_size) {
	image = img;
	if (auto_size) {
		auto imgsize = image.getSize();
		setSize({ (float)imgsize.x, (float)imgsize.y }, true);
	}
	return update();
}

void ui::Image::render(sf::RenderWindow& rw) {
	sprite.setScale({
		getSize().x / (float)image.getSize().x,
		getSize().y / (float)image.getSize().y
	});
	rw.draw(sprite);
}
