#include "gui/gui.h"
#include "gui/helpers.h"

#include "gui/roundrectangle.h"

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

struct TestElement : ui::InlineElement {
	TestElement(std::string name) : name(name) {
		std::cout << name << " was constructed" << std::endl;
		changeColor();
		label = add<ui::Text>(name, getFont());
		add<ui::CallbackButton>("Change Colour", getFont(), [this]{
			changeColor();
		});
		add<ui::PullDownMenu<std::string>>(std::vector<std::string>{"Bread", "Butter", "Apricots", "Orphans", "Oregonians", "Orifices", "Mankind"},
										   getFont(),
										   [this](const std::string& s){
			this->label->setText(s);
		});
		std::vector<std::pair<sf::Color, std::string>> color_options {
			{sf::Color(0xcf302dff), "red"},
			{sf::Color(0x990f04ff), "cherry"},
			{sf::Color(0xe2252bff), "rose"},
			{sf::Color(0x600f0cff), "jam"},
			{sf::Color(0x600f0cff), "merlot"},
			{sf::Color(0x5f0a03ff), "garnet"},
			{sf::Color(0xb80f0aff), "crimson"},
			{sf::Color(0x900604ff), "ruby"},
			{sf::Color(0x920c09ff), "scarlet"},
			{sf::Color(0x4c0805ff), "wine"},
			{sf::Color(0x7e2811ff), "brick"},
			{sf::Color(0xa91b0dff), "apple"},
			{sf::Color(0x420d09ff), "mahogany"},
			{sf::Color(0x720d05ff), "blood"},
			{sf::Color(0x5e1915ff), "sangria"},
			{sf::Color(0x7a1913ff), "berry"},
			{sf::Color(0x670c09ff), "currant"},
			{sf::Color(0xbc5449ff), "blush"},
			{sf::Color(0xd31505ff), "candy"},
			{sf::Color(0x9c1003ff), "lipstick"}
		};
		add<ui::PullDownMenu<sf::Color>>(color_options, getFont(), [this](const sf::Color& color){
			this->bgcolor = color;
		});

		add<ui::TextEntryHelper>("", getFont(), [this](const std::wstring& text){
			this->label->setText(text);
		});

		float val = 0.5f;
		float min = 0.0f;
		float max = 1.0f;
		auto numbrocont = add<ui::InlineElement>();
		numbrocont->setSize({100.0f, 35.0f}, true);
		auto numbro = numbrocont->add<ui::Text>(std::to_string(val), getFont());
		auto onChange = [numbro](float v){
			numbro->setText(std::to_string(v));
		};
		add<ui::NumberTextEntry>(val, min, max, getFont(), onChange);
		add<ui::Slider>(val, min, max, getFont(), onChange);
	}
	~TestElement(){
		std::cout << name << " was destroyed" << std::endl;
	}

	void onClose() override {
		std::cout << name << " was closed" << std::endl;
	}

	void changeColor(){
		std::uniform_int_distribution<unsigned> dist(0, 0xFF);
		bgcolor = sf::Color(dist(randeng),
							dist(randeng),
							dist(randeng),
							255);
	}

	void render(sf::RenderWindow& rw){
		ui::RoundedRectangle rect;
		rect.setSize(getSize());
		rect.setRadius(getPadding());
		rect.setFillColor(bgcolor);
		rect.setOutlineColor(sf::Color(0xFF));
		rect.setOutlineThickness(1.0f);
		rw.draw(rect);
	}

	bool onLeftClick(int clicks) override {
		bringToFront();
		if (clicks == 1){
			std::cout << name << " was left-clicked" << std::endl;
			setDisplayStyle(DisplayStyle::Free);
			startDrag();
		} else if (clicks == 2){
			std::cout << name << " was left-clicked twice" << std::endl;
			changeColor();
		}
		return true;
	}

	void onLeftRelease() override {
		std::cout << name << " was left released" << std::endl;
		if (dragging()){
			stopDrag();
			setDisplayStyle(DisplayStyle::Inline);
			drop(localMousePos());
		}
	}

	bool onRightClick(int clicks) override {
		if (clicks == 1){
			std::cout << name << " was right-clicked once" << std::endl;
			std::uniform_real_distribution<float> sdist {20, 300};
			std::uniform_real_distribution<float> mdist {0, 20};
			auto self = shared_from_this();
			vec2 oldsize = getSize();
			vec2 newsize = {sdist(randeng), sdist(randeng)};
			float oldmargin = getMargin();
			float newmargin = mdist(randeng);
			float oldpadding = getPadding();
			float newpadding = mdist(randeng);
			ui::startTransition(1.0, [=](float t){
				float x = 0.5f - 0.5f * cos(t * 3.141592654f);
				self->setSize(oldsize * (1.0f - x) + newsize * x);
				self->setMargin(oldmargin * (1.0f - x) + newmargin * x);
				self->setPadding(oldpadding * (1.0f - x) + newpadding * x);
			});
		} else if (clicks == 2){
			std::cout << name << " was right-clicked twice" << std::endl;
		}
		return true;
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

	bool onKeyDown(ui::Key key) override {
		std::cout << name << " - [" << key << "] was pressed" << std::endl;
		if (key == ui::Key::Escape){
			close();
		}
		return true;
	}

	void onKeyUp(ui::Key key) override {
		std::cout << name << " - [" << key << "] was released" << std::endl;
	}

	bool onScroll(float dx, float dy) override {
		std::cout << name << " was scrolled (" << dx << ", " << dy << ')' << std::endl;
		return true;
	}

	void onDrag() override {

	}

	bool onHover() override {
		return true;
	}

	bool onHoverWith(std::shared_ptr<Element> element) override {
		return true;
	}

	bool onDrop(std::shared_ptr<Element> element) override {
		if (element){
			if (auto w = std::dynamic_pointer_cast<TestElement, Element>(element)){
				adopt(w);
				std::cout << w->name;
			} else {
				std::cout << "An element";
			}
		} else {
			std::cout << "Mystery???";
		}
		std::cout << " was dropped onto " << name << std::endl;
		return true;
	}

	sf::Color bgcolor;
	const std::string name;
	std::shared_ptr<ui::Text> label;
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
	}
};

int main(int argc, char** argcv){

	ui::init(1000, 800, "Tim's GUI Test", 30);

	{
		auto b = ui::root().add<ui::BlockElement>();
		for (int i = 0; i < 5; ++i){
			b->add<ui::Text>("Block " + std::to_string(i + 1), getFont());
			auto newb = b->add<ui::BlockElement>();
			b->add<ui::Text>("Block " + std::to_string(i + 1), getFont());
			b = newb;
		}

		auto block = ui::root().add<ui::BlockElement>();
		block->setMinSize({0, 100});
		auto pops = block->add<TestElement>("Pops");
		pops->setMinSize({500, 300});
		pops->add<TestElement>("Hector");
		pops->add<TestElement>("Brent");
		pops->add<TestElement>("Greg");
		pops->add<TestElement>("Donny");

		block->add<ui::LineBreak>(10.0f);

		block->add<TestElement>("Jorgan");

		block->add<ui::LineBreak>(15.0f);

		block->add<TestElement>("Allen");;
		block->add<TestElement>("Percy");
		block->add<TestElement>("Collin");
		block->add<TestElement>("Geoffrey");
		block->add<TestElement>("Hank");
		block->add<TestElement>("Brody");

		std::wstringstream ss;
		ss.str(L"Lorem ipsum dolor sit amet, ne choro legendos expetendis quo. Ei mel nibh dissentiunt, ius nibh nobis ei, at mel feugiat platonem. Et hinc graeco veritus pro. Liber inimicus repudiare ex usu. Ad nec evertitur sadipscing, id oratio legere nec. Ad eum eros congue phaedrum, eos nonumy phaedrum ut, soluta interpretaris ad nam. Sed tation sensibus constituam te. Vel altera legimus no, sit vide modus neglegentur ad, ocurreret laboramus disputando ad eum. Laoreet convenire ei vis. At sed agam mollis blandit, ex noster facete ius. Nobis denique vix ei. Ea sumo invenire per, tempor integre an usu, at soluta nostrud signiferumque his. Ex feugait quaestio vel, nonumy prompta ullamcorper vel in. Ea rebum posse constituto quo. Ex nostro malorum eleifend vel. Etiam verterem splendide vel ut, his no tantas commune. Sea cu solet detracto, mei propriae neglegentur eu. Cum ad quas singulis iudicabit, erat adolescens id qui, mel in quem sadipscing. Eu duo eius neglegentur, vix debet mediocrem in, id graece sensibus est. Ex sea veniam omnium veritus, an mea scaevola efficiendi. Duo minim maluisset te, ne qui democritum sadipscing. Eu rebum voluptaria ullamcorper quo. Ei est verterem imperdiet, his delicata vituperata te. Ei utinam insolens temporibus duo, et vis ancillae voluptaria. His clita doctus minimum at. Usu no mutat timeam assueverit, nobis mnesarchum sadipscing at cum. An illud minim nec, no errem dicunt accusamus pro, ad sanctus docendi delicata mel.");
		std::wstring word;
		while (ss >> word){
			block->add<ui::Text>(word, getFont());
		}
	}

	ui::run();

	return 0;
}