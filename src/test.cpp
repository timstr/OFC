#include "gui/gui.h"
#include <iostream>
#include <random>

std::random_device randdev;
std::mt19937 randeng {randdev()};

struct TestWindow : ui::Window {
	TestWindow(){
		std::cout << "Constructed" << std::endl;
		size = {100, 100};
		changeColor();
	}
	~TestWindow(){
		std::cout << "Destroyed" << std::endl;
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
	}

	void onLeftClick(int clicks) override {
		if (clicks == 1){
			std::cout << "Left-clicked once" << std::endl;
			startDrag();
		} else if (clicks == 2){
			std::cout << "Left-clicked twice" << std::endl;
			changeColor();
		}
	}

	void onLeftRelease() override {
		bringToFront();
		std::cout << "Left released" << std::endl;
		stopDrag();
	}

	void onRightClick(int clicks) override {
		if (clicks == 1){
			std::cout << "Right-clicked once" << std::endl;
		} else if (clicks == 2){
			std::cout << "Right-clicked twice" << std::endl;
		}
	}

	void onRightRelease() override {
		std::cout << "Right released" << std::endl;
	}

	void onFocus() override {
		std::cout << "Focused" << std::endl;
	}

	void onLoseFocus() override {
		std::cout << "Lost focus" << std::endl;
	}

	void onKeyDown(ui::Key key) override {
		std::cout << '[' << key << "] was pressed" << std::endl;
		if (key == ui::Key::Escape){
			close();
		}
	}

	void onKeyUp(ui::Key key) override {
		std::cout << '[' << key << "] was released" << std::endl;
	}

	void onScroll(float dx, float dy) override {
		std::cout << "Scrolled (" << dx << ", " << dy << ')' << std::endl;
	}

	void onDrag() override {

	}

	void onHover() override {

	}

	void onHoverWithWindow(std::weak_ptr<Window> window) override {
		std::cout << "Hovering with a window" << std::endl;
	}

	bool onDropWindow(std::weak_ptr<Window> window) override {
		std::cout << "A window was dropped" << std::endl;
		return true;
	}

	sf::Color bgcolor;
};

int main(int argc, char** argcv){

	ui::init(500, 400, "Tim's GUI Test", 30);

	ui::root().add<TestWindow>(vec2(10, 10));
	ui::root().add<TestWindow>(vec2(120, 120));

	ui::run();

	return 0;
}