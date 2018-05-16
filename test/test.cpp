#include "gui/gui.h"
#include <iostream>
#include <random>

#include "fontspath.h"

std::random_device randdev;
std::mt19937 randeng {randdev()};

sf::Font& getFont(){
	static sf::Font font;
	static bool loaded;
	if (!loaded){
		font.loadFromFile(fonts_path + "/JosefinSans-Bold.ttf");
		loaded = true;
	}
	return font;
}

struct TestWindow : ui::Window {
	TestWindow(std::string name) : name(name) {
		std::cout << name << " was constructed" << std::endl;
		size = {100, 100};
		changeColor();
		add<ui::Text>(name, getFont());
	}
	~TestWindow(){
		std::cout << name << " was destroyed" << std::endl;
	}

	void changeColor(){
		std::uniform_int_distribution<unsigned> dist(0, 0xFF);
		bgcolor = sf::Color(dist(randeng),
							dist(randeng),
							dist(randeng),
							255);
	}

	void render(sf::RenderWindow& rw){
		sf::RectangleShape rect {size};
		rect.setFillColor(bgcolor);
		rw.draw(rect);
		renderChildWindows(rw);
	}

	void onLeftClick(int clicks) override {
		bringToFront();
		if (clicks == 1){
			std::cout << name << " was left-clicked once" << std::endl;
			startDrag();
		} else if (clicks == 2){
			std::cout << name << " was left-clicked twice" << std::endl;
			changeColor();
		}
	}

	void onLeftRelease() override {
		std::cout << name << " was left released" << std::endl;
		stopDrag();
		drop(localMousePos());
	}

	void onRightClick(int clicks) override {
		if (clicks == 1){
			std::cout << name << " was right-clicked once" << std::endl;
		} else if (clicks == 2){
			std::cout << name << " was right-clicked twice" << std::endl;
		}
	}

	void onRightRelease() override {
		std::cout << name << " was right released" << std::endl;
	}

	void onFocus() override {
		std::cout << name << " gained focus" << std::endl;
	}

	void onLoseFocus() override {
		std::cout << name << " lost focus" << std::endl;
	}

	void onKeyDown(ui::Key key) override {
		std::cout << name << " - [" << key << "] was pressed" << std::endl;
		if (key == ui::Key::Escape){
			close();
		}
	}

	void onKeyUp(ui::Key key) override {
		std::cout << name << " - [" << key << "] was released" << std::endl;
	}

	void onScroll(float dx, float dy) override {
		std::cout << name << " was scrolled (" << dx << ", " << dy << ')' << std::endl;
	}

	void onDrag() override {

	}

	void onHover() override {

	}

	void onHoverWithWindow(std::weak_ptr<Window> window) override {
		
	}

	bool onDropWindow(std::weak_ptr<Window> window) override {
		if (auto win = window.lock()){
			if (auto w = std::dynamic_pointer_cast<TestWindow, Window>(win)){
				std::cout << w->name << " was dropped";
			} else {
				std::cout << "A window was dropped";
			}
		} else {
			std::cout << "Mystery???";
		}
		std::cout << " was dropped onto " << name << std::endl;
		return true;
	}

	sf::Color bgcolor;
	const std::string name;
};

int main(int argc, char** argcv){

	ui::init(500, 400, "Tim's GUI Test", 30);

	ui::root().add<TestWindow>(vec2(10, 10), "Hector");
	ui::root().add<TestWindow>(vec2(20, 20), "Brent");
	ui::root().add<TestWindow>(vec2(30, 30), "Arnold");
	ui::root().add<TestWindow>(vec2(40, 40), "Jason");
	ui::root().add<TestWindow>(vec2(50, 50), "Greg");

	ui::root().add<ui::TextEntry>(vec2(200, 200), "Enter text...", getFont());

	ui::run();

	return 0;
}