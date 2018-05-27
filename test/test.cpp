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

struct TestElement : ui::FreeElement {
	TestElement(vec2 position, std::string name) : name(name) {
		std::cout << name << " was constructed" << std::endl;
		setSize({100, 100});
		setPos(position);
		changeColor();
		add<ui::Text>(name, getFont());
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
		sf::RectangleShape rect {getSize()};
		rect.setFillColor(bgcolor);
		rw.draw(rect);
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
		} else if (key == ui::Key::Space){
			std::uniform_real_distribution<float> fdist {20, 200};
			auto self = shared_from_this();
			vec2 oldsize = getSize();
			vec2 newsize = {fdist(randeng), fdist(randeng)};
			ui::startTransition(1.0, [=](float t){
				self->setSize(oldsize * (1.0f - t) + newsize * t);
			});
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

	void onHoverWith(std::weak_ptr<Element> element) override {
		
	}

	bool onDrop(std::weak_ptr<Element> element) override {
		if (auto elem = element.lock()){
			if (auto w = std::dynamic_pointer_cast<TestElement, Element>(elem)){
				std::cout << w->name << " was dropped";
			} else {
				std::cout << "An element was dropped";
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

	std::uniform_int_distribution<int> dist{0, 5};
	
	auto block = std::make_shared<ui::BlockElement>();

	ui::root().add(block);

	std::wstringstream ss;
	ss.str(L"It was a dark cold night. Two armies faced each other  inlocked in the furry of combat. Beside a flowing river dilluted red with bolood. A man stood wisping his sword at another, the other man a wood shild incresed with a purple tree wearing a crown. The first man slashed his sword at the others legs - blocked. He deflected a stab at his ribs and started a jumping slash at the man's head. The man held his sword above head in antisipation. But it never came. At the last moment he switched and stabbed the man in the ribs. Ruby red blood seaped from the man's chest his face changed to unexpexted aginey. Not leting him recover he slashed off his head, steping over the lutenits dead body. He lifted his sword and roared in victory. But he had defeted one lesser man. Suddenly, he was surounded. Slashing there swords in a frensie they over powered him. But they had nd success he slashed one in the throught, stabed one in the heart and slashed at another ones back. The rest ran from his glisening blade. He charged. He cought one and killed him with a stab throw the back. He glanced behind him. His arme was scattered and small. \"How can we will\" he thought. He looked at there arme. There was a man screaming orders. He gathered a number of his men from the fray. \"Follow me\" he wispered. \"Take out the men in the way\" \"charge\" he screamed. The surprised solders fell to there blades. The man in the middle looked at him. A lothing glint in his dark brown eyes. He drew his saber slowly as it glitered with reflected light. Becining him towards him he walked out at him.  Quickly he sprinted and slashed at his legs. The man swiftly parried his blow and slashed at his back. He rolled to avoid the the blow and slashed at his calf. The man was slow. His calf was slised. He snurled. The other man stabed at his ribs but stumbled and fell on the ground. A sword was at his neck. Quickly he kicked the  man in the shing grabed his fallen sword and attacked. His blade wisled throw the air. The other man looked at the sharp piece of metal flying towards his face and ducked. Surprised, the other man's swords momentum made him lose balance. That was all his enemy needed slashing at his left leg. As fast as a hawk he parried the blow with his sword. Grabbing a forgotten sheild he hit the the man in the face and cut off his sword arm. The man colapst his face covered with blood he looked with horror at his lost limb. As he was stabed in the heart. The other man pulled his sword from the corpse and looked for his men. Only two were left. They were figuting a group of five men. He ran at them shocked one looked over only to be killed by a sword. The rest kept fighting but fear was clearly written on there faces. He arived at there death bed and caved ones helmit in and beheaded beheaded him. The last three ran to a beater spot \"attack them\" he yeld. They sprinted at them. He slashed one in the legs dyeing his blade red. And finished him off. He charged at another.");

	std::uniform_int_distribution<int> wordist {1, 15};
	while (ss){
		std::wstring word;
		auto ib = std::make_shared<ui::InlineElement>();
		int len = wordist(randeng);
		for (int i = 0; i < len && (bool)(ss >> word); ++i){
			auto w = std::make_shared<ui::Text>(word, getFont());
			ib->add(w);
		}
		ib->setMinSize({250, 0});
		block->add(ib);
	}

	block = std::make_shared<ui::BlockElement>();
	block->setMinSize({200, 200});
	block->add<TestElement>(vec2(10, 10), "Hector");
	block->add<TestElement>(vec2(20, 20), "Brent");
	block->add<TestElement>(vec2(30, 30), "Greg");
	block->add<TestElement>(vec2(40, 40), "Donny");
	block->add<TestElement>(vec2(50, 50), "Jorgan");
	block->add<TestElement>(vec2(60, 60), "Allen");
	block->add<TestElement>(vec2(70, 70), "Percy");
	ui::root().add(block);

	for (int i = 0; i < 50; ++i){
		if (dist(randeng) == 0){
			auto guy = std::make_shared<RedMan>();
			ui::root().add(guy);
		} else {
			auto guy = std::make_shared<BlueMan>();
			ui::root().add(guy);
		}
	}

	ui::run();

	return 0;
}