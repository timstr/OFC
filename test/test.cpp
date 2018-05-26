#include "gui/gui.h"
#include <iostream>
#include <random>
#include <sstream>

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

struct TestWindow : ui::FreeElement {
	TestWindow(std::string name) : name(name) {
		std::cout << name << " was constructed" << std::endl;
		setSize({100, 100});
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
		sf::RectangleShape rect {getSize()};
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

struct BlueMan : ui::InlineElement {
	BlueMan(){
		std::uniform_int_distribution<int> xdist {30, 200};
		std::uniform_int_distribution<int> ydist {20, 50};

		setMinSize({(float)xdist(randeng), (float)ydist(randeng)});

		add<ui::Text>("Inline", getFont());
	}

	void render(sf::RenderWindow& rw) override {
		sf::RectangleShape rect(getSize());
		rect.setFillColor(sf::Color(0x0000FF80));
		rw.draw(rect);
		renderChildWindows(rw);
	}
};

struct RedMan : ui::BlockElement {
	RedMan(){
		std::uniform_int_distribution<int> xdist {50, 200};
		std::uniform_int_distribution<int> ydist {20, 50};

		setMinSize({(float)xdist(randeng), (float)ydist(randeng)});

		add<ui::Text>("Block", getFont());
	}

	void render(sf::RenderWindow& rw) override {
		sf::RectangleShape rect(getSize());
		rect.setFillColor(sf::Color(0xFF000080));
		rw.draw(rect);
		renderChildWindows(rw);
	}
};

int main(int argc, char** argcv){

	ui::init(1000, 800, "Tim's GUI Test", 30);

	//ui::root().add<TestWindow>(vec2(10, 10), "Hector");
	//ui::root().add<TestWindow>(vec2(20, 20), "Brent");
	//ui::root().add<TestWindow>(vec2(50, 50), "Greg");

	//ui::root().add<ui::TextEntry>(vec2(600, 200), "Enter text...", getFont());

	std::uniform_int_distribution<int> dist{0, 5};
	
	auto block = std::make_shared<ui::BlockElement>();

	ui::root().add(block);

	for (int i = 0; i < 50; ++i){
		auto nextblock = std::make_shared<ui::BlockElement>();
		block->add(nextblock);
		block = nextblock;
	}
	
	std::stringstream ss;
	ss.str("Jeremiah was a bullfrog");
	std::string word;
	while (ss >> word){
		block->add<ui::Text>(word, getFont());
	}

	for (int i = 0; i < 50; ++i){
		if (dist(randeng) == 0){
			ui::root().add<RedMan>();
		} else {
			ui::root().add<BlueMan>();
		}
	}

	ui::addKeyboardCommand(ui::Key::U, []{
		ui::root().update(ui::root().getSize().x);
	});

	ui::run();

	return 0;
}