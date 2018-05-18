#include "gui/gui.h"
#include <iostream>
#include <random>
#include "gui/divider.h"
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

struct TestWindow : ui::Window {
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

struct BlueMan : ui::Window {
	BlueMan(){
		std::uniform_real_distribution<float> xdist {30.0f, 200.0f};
		std::uniform_real_distribution<float> ydist {20.0f, 50.0f};

		setSize({xdist(randeng), ydist(randeng)});
		display_style = DisplayStyle::Inline;

		add<ui::Text>("Inline", getFont());
	}

	void render(sf::RenderWindow& rw) override {
		sf::RectangleShape rect(getSize());
		rect.setFillColor(sf::Color(0x0000FF80));
		rw.draw(rect);
		renderChildWindows(rw);
	}
};

struct RedMan : ui::Window {
	RedMan(){
		std::uniform_real_distribution<float> xdist {50.0f, 200.0f};
		std::uniform_real_distribution<float> ydist {20.0f, 50.0f};

		setSize({xdist(randeng), ydist(randeng)});
		display_style = DisplayStyle::Block;

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

	auto div = std::make_shared<ui::Divider>();
	div->setSize({300, 300});
	div->clipping = true;

	std::uniform_int_distribution<int> dist {0, 5};

	for (int i = 0; i < 10; i++){
		if (dist(randeng) > 0){
			div->add<BlueMan>();
		} else {
			div->add<RedMan>();
		}
	}

	auto div2 = std::make_shared<ui::Divider>();
	div2->add<RedMan>();
	for (int i = 0; i < 10; i++){
		div2->add<BlueMan>();
	}
	div2->add<RedMan>();

	auto nudiv = std::make_shared<ui::Divider>();
	div2->add(nudiv);
	for (int i = 0; i < 10; i++){
		auto nextdiv = std::make_shared<ui::Divider>();
		nudiv->add(nextdiv);
		nudiv = nextdiv;
	}
	
	std::wstringstream ss;
	ss.str(L"I love my pet dog, Sparky. One day it ate my parrot and I had to clean the mess off the mezzanine.");

	std::wstring word;
	while (ss >> word){
		auto text = std::make_shared<ui::Text>(word, getFont());
		text->display_style = ui::Window::DisplayStyle::Inline;
		nudiv->add(text);
	}

	div->add(div2);

	ss.clear();
	ss.str(L"So Robert huwe was rocking to some stiffiling music when his cousin came home. “ damn it huwe I told you to stop stuffing the chicken when it is still alive.” “Fuck whistle” Huwe said as he ran down stairs with the cast iron shovel. “ I got this” he said rapidity hitting the chicken on the bonk for a good solid chunk of his life till its dead steaming body lay deceased on the broken crusted floor boards. “is that good enough” Huwe asked. “No fucking damn it huwe its to dead.” His cousin said pick’n up the shovel to continue the beating. “well I’ll get the potatoes then” he said as he walked to the store. As he arrived the typical customers had all run dry leaving only him and his sweet appetite in the coffee shop. “Could I have some potatoes” he asked the nice cranky woman running the cash machine. “no” she said. “ok then” he said walking to trade his appetite for some other limited time appetizer. He came back to his house in one hell of a mood the yack fries were out the camel’s toes was gone and all the loons were fresh out of stock. As he strutted in to the house with a passion he found his cousin lying dead asleep on the floor. He looks around to see that the chicken was still clucking in the microwave.  “That chicken was one stubborn goose” he said putting tinfoil in there with it. After the lighting stopped he removed it and started eating it without pause and days and a night passed before he asked himself. “Where are the potatoes?” He stormed out into the crunchy highway degenerated by the lack of anger in his heart. After he arrived in his local coffee shop he was alone with not even the cranky widow to accompany him. “Could I have a potato.” He asked. “No” he said to himself. He walked over the gently sleeping body at his feet to return home. “Well it beats me.” Said huwe. As the flap they used as a door flapped violently in the wind huwe returned to the tent he called home to celebrate his birthday in two and a half months. “Now that’s what I call a party.” huwe said enthusiastically. As the last rain drop of the sea rain fell for the season. He realized what a simpleton he had been he had forgot the potatoes. “well it doesn’t matter anymore”  he stated.");
	while (ss >> word){
		auto text = std::make_shared<ui::Text>(word, getFont());
		text->display_style = ui::Window::DisplayStyle::Inline;
		div->add(text);
	}

	ui::root().add(div);

	div->setPos({10, 10});

	ui::addKeyboardCommand(ui::Key::Space, [&]{
		std::uniform_real_distribution<float> xdist {100.0f, 1000.0f};
		std::uniform_real_distribution<float> ydist {100.0f, 1000.0f};
		div->setSize({xdist(randeng), ydist(randeng)});
	});

	ui::addKeyboardCommand(ui::Key::LShift, [&]{
		vec2 size;
		size = ui::getMousePos() - div->rootPos();
		div->setSize(size);
	});

	ui::run();

	return 0;
}