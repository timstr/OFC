#include <GUI/GUI.hpp>
#include <GUI/Helpers.hpp>

#include <iostream>
#include <random>
#include <sstream>

#include "fontspath.hpp"

std::random_device randdev;
std::mt19937 randeng { randdev() };

const sf::Font& getFont() {
	static sf::Font font;
	static bool loaded;
	if (!loaded) {
		font.loadFromFile(fonts_path + "/JosefinSans-Bold.ttf");
		loaded = true;
	}
	return font;
}

struct TestElement : ui::Control, ui::Container, ui::BoxElement, ui::Draggable {
	TestElement(ui::String name) : name(name), label(add<ui::Text>(name, getFont())) {

		std::cout << static_cast<std::string>(name) << " was constructed" << std::endl;
		changeColor();

		auto& colorbtn = add<ui::CallbackButton>("Change Colour", getFont(), [this] {
			changeColor();
		});


		colorbtn.setNormalColor(0xFF0000FF);
		colorbtn.setActiveColor(0x00FF00FF);
		colorbtn.setHoverColor(0x0000FFFF);

		add<ui::PullDownMenu<ui::String>>(
            std::vector<ui::String>{"Bread", "Butter", "Apricots", "Orphans", "Oregonians", "Orifices", "Mankind"},
            getFont(),
            [this](const ui::String& s) {
                this->label.setText(s);
		    }
        );
		std::vector<std::pair<ui::Color, ui::String>> color_options {
			{0xcf302dff, "red"},
			{0x990f04ff, "cherry"},
			{0xe2252bff, "rose"},
			{0x600f0cff, "jam"},
			{0x600f0cff, "merlot"},
			{0x5f0a03ff, "garnet"},
			{0xb80f0aff, "crimson"},
			{0x900604ff, "ruby"},
			{0x920c09ff, "scarlet"},
			{0x4c0805ff, "wine"},
			{0x7e2811ff, "brick"},
			{0xa91b0dff, "apple"},
			{0x420d09ff, "mahogany"},
			{0x720d05ff, "blood"},
			{0x5e1915ff, "sangria"},
			{0x7a1913ff, "berry"},
			{0x670c09ff, "currant"},
			{0xbc5449ff, "blush"},
			{0xd31505ff, "candy"},
			{0x9c1003ff, "lipstick"}
		};
		auto& redmenu = add<ui::PullDownMenu<ui::Color>>(color_options, getFont(), [this](const ui::Color& color) {
			this->setBackgroundColor(color);
		});

		redmenu.setNormalColor(0xFF0000FF);
		redmenu.setActiveColor(0x00FF00FF);
		redmenu.setHoverColor(0x0000FFFF);

		add<ui::CallbackTextEntry>("", getFont(), [this](const ui::String& text) {
			this->label.setText(text);
		});
	}
	~TestElement() {
		std::cout << name << " was destroyed" << std::endl;
	}
    
	void changeColor() {
		std::uniform_int_distribution<unsigned> dist(0, 0xFF);
		setBackgroundColor(ui::Color(
			(uint8_t)dist(randeng),
			(uint8_t)dist(randeng),
			(uint8_t)dist(randeng),
			255));
	}

	bool onLeftClick(int clicks) override {
		if (clicks == 1) {
			std::cout << name << " was left-clicked" << std::endl;
			startDrag();
		} else if (clicks == 2) {
			std::cout << name << " was left-clicked twice" << std::endl;
			changeColor();
		}
		return true;
	}

	void onLeftRelease() override {
		std::cout << name << " was left released" << std::endl;
		if (dragging()) {
			stopDrag();
			drop(localMousePos());
		}
	}

	bool onRightClick(int clicks) override {
		if (clicks == 1) {
			std::cout << name << " was right-clicked once" << std::endl;
			std::uniform_int_distribution<int> sdist { 20, 300 };
			std::uniform_int_distribution<int> mdist { 0, 20 };
			ui::vec2 oldsize = size();
			ui::vec2 newsize = { (float)sdist(randeng), (float)sdist(randeng) };
			startTransition(1.0, [=](float t) {
				float x = 0.5f - 0.5f * cos(t * 3.141592654f);
				setSize(oldsize * (1.0f - x) + newsize * x);
			});
		} else if (clicks == 2) {
			std::cout << name << " was right-clicked twice" << std::endl;
		}
		return true;
	}

	void onRightRelease() override {
		std::cout << name << " was right released" << std::endl;
	}

	bool onMiddleClick(int clicks) override {
		std::cout << name << " was " << (clicks == 2 ? "double-" : "") << "middle clicked" << std::endl;
		return true;
	}

	void onMiddleRelease() override {
		std::cout << name << " was middle released" << std::endl;
	}

	void onGainFocus() override {
		std::cout << name << " gained focus" << std::endl;
	}

	void onLoseFocus() override {
		std::cout << name << " lost focus" << std::endl;
	}

	bool onKeyDown(ui::Key key) override {
		std::cout << name << " - [" << key << "] was pressed" << std::endl;
		if (key == ui::Key::Delete) {
			close();
		}
		return true;
	}

	void onKeyUp(ui::Key key) override {
		std::cout << name << " - [" << key << "] was released" << std::endl;
	}

	bool onScroll(ui::vec2 d) override {
		std::cout << name << " was scrolled (" << d.x << ", " << d.y << ')' << std::endl;
		return true;
	}

	bool onDrop(Draggable* d) override {
		if (d) {
			if (auto w = dynamic_cast<TestElement*>(d)) {
				adopt(w->orphan());
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

    void render(sf::RenderWindow& rw) override {
        ui::BoxElement::render(rw);
        ui::Container::render(rw);
    };

	const std::string name;
	ui::Text& label;

	struct SizeButton : ui::Control, ui::BoxElement, ui::Draggable {
		SizeButton(TestElement& _parent) : parent(_parent) {
			setSize({ 20.0f, 20.0f }, true);
			setBackgroundColor(0x808080FF);
			setBorderThickness(0.0f);
		}

		bool onLeftClick(int) override {
			startDrag();
			return true;
		}

		void onLeftRelease() override {
			stopDrag();
		}

		void onDrag() override {
			parent.setSize(pos() + size());
		}

		TestElement& parent;
	};
};

struct BoxContainer : ui::FreeContainer, ui::BoxElement {
    void render(sf::RenderWindow& rw) override {
        ui::BoxElement::render(rw);
        ui::Container::render(rw);
    }
};

void makeGridLayout(ui::Window& win){
    auto& cont = win.setRoot<ui::GridContainer>(3, 3);
    //auto& cont = root.add<ui::GridContainer>(ui::PositionStyle::Center, ui::PositionStyle::Center, 3, 3);
    //cont.setSize({300.0f, 300.0f}, true);
    cont.setRowHeight(0, 1.0f);
    cont.setRowHeight(1, 2.0f);
    cont.setRowHeight(2, 4.0f);
    cont.setColumnWidth(0, 5.0f);
    cont.setColumnWidth(1, 1.0f);
    cont.setColumnWidth(2, 1.0f);

    const auto add_cell = [&cont](size_t i, size_t j, ui::Color color){
        auto& c = cont.putCell<BoxContainer>(i, j);
        c.setHorizontalFill(true);
        c.setVerticalFill(true);
        c.setBackgroundColor(color);
        auto& c2 = c.add<BoxContainer>(ui::PositionStyle::Center, ui::PositionStyle::Center);
        c2.setBackgroundColor(0xFFFFFFFF);
        c2.setSize({25.0f, 25.0f}, true);
        c.add<ui::Text>(ui::PositionStyle::Center, ui::PositionStyle::Center, "X", getFont());
    };
    
    add_cell(0, 0, 0x000000FF);
    add_cell(0, 1, 0x880000FF);
    add_cell(0, 2, 0x000000FF);
    add_cell(1, 0, 0x880000FF);
    add_cell(1, 1, 0x000000FF);
    add_cell(1, 2, 0x880000FF);
    add_cell(2, 0, 0x000000FF);
    add_cell(2, 1, 0x880000FF);
    add_cell(2, 2, 0x000000FF);
}

class DragButton : public ui::Draggable, public ui::Control, public ui::BoxElement {
public:
    DragButton(){
        setSize({100, 100});
        setBackgroundColor(0x008000FF);
    }

    bool onLeftClick(int clicks) override {
        startDrag();
        return true;
    }

    void onLeftRelease() override {
        stopDrag();
    }
};

int main() {
    
    ui::Window& win = ui::Window::create(1000, 800, "Tim's GUI Test");

    // makeGridLayout(win);

    auto& root = win.setRoot<ui::FreeContainer>();

    root.add<DragButton>();
    
    /*auto& elem1 = root.add<BoxContainer>();
    elem1.setPos({50, 50});
    elem1.setBackgroundColor(0xFFFFFF40);


    auto child1 = std::make_unique<BoxContainer>();
    child1->write("Child 1", getFont());
    child1->setBackgroundColor(0xFF0000FF);

    auto child2 = std::make_unique<BoxContainer>();
    child2->write("Child 2", getFont());
    child2->setBackgroundColor(0x00FF00FF);

    auto child3 = std::make_unique<BoxContainer>();
    child3->write("Child 3", getFont());
    child3->setBackgroundColor(0x0000FFFF);

    elem1.adopt(std::move(child1));
    elem1.adopt(std::move(child2));
    elem1.adopt(std::move(child3));*/



	/*{
		auto block = ui::root().add<ui::BlockElement>();
		block->setBackgroundColor(sf::Color(0xFFFFFFFF));

		block->setMinSize({ 0, 100 });

		auto pops = block->add<TestElement>("Pops");
		pops->setLayoutStyle(ui::LayoutStyle::FloatLeft);
		pops->add<TestElement>("Hector");

		block->add<TestElement>("Hank");

		block->setContentAlign(ui::ContentAlign::Justify);

		auto widg = block->add<ui::RightFloatingElement>();
		widg->setMargin(5.0f);
		widg->setBorderThickness(1.0f);
		widg->setBorderRadius(5.0f);
		widg->add<ui::Text>("Table of Contents", getFont(), sf::Color(0x404040FF), 30, ui::TextStyle::Underlined);
		auto table_o_contents = widg->add<ui::BlockElement>();
		table_o_contents->writePageBreak();
		table_o_contents->add<ui::Text>("Introduction", getFont(), sf::Color(0x000040FF), 15, ui::TextStyle::Underlined)->setLayoutStyle(ui::LayoutStyle::FloatRight);
		table_o_contents->writePageBreak();
		table_o_contents->add<ui::Text>("First Paragraph", getFont(), sf::Color(0x000040FF), 15, ui::TextStyle::Underlined)->setLayoutStyle(ui::LayoutStyle::FloatRight);
		table_o_contents->writePageBreak();
		table_o_contents->add<ui::Text>("Last Paragraph", getFont(), sf::Color(0x000040FF), 15, ui::TextStyle::Underlined)->setLayoutStyle(ui::LayoutStyle::FloatRight);
		table_o_contents->writePageBreak();
		table_o_contents->add<ui::Text>("Conclusion", getFont(), sf::Color(0x000040FF), 15, ui::TextStyle::Underlined)->setLayoutStyle(ui::LayoutStyle::FloatRight);
		table_o_contents->writePageBreak();
		int count = 1;
		std::shared_ptr<ui::CallbackButton> btn = widg->add<ui::CallbackButton>("Add a guy", getFont(), [&, table_o_contents]() {
			auto par = table_o_contents->add<ui::Text>("Bonus Paragraph " + std::to_string(count), getFont(), sf::Color(0x000040FF), 15, ui::TextStyle::Underlined);
			table_o_contents->writePageBreak();
			par->setLayoutStyle(ui::LayoutStyle::FloatRight);
			par->layoutBefore(btn);
			++count;
		});

		ui::String str = L"\tLorem ipsum dolor sit amet, ne choro legendos expetendis quo. Ei mel nibh dissentiunt, ius nibh nobis ei, at mel feugiat platonem. Et hinc graeco veritus pro. Liber inimicus repudiare ex usu. Ad nec evertitur sadipscing, id oratio legere nec. Ad eum eros congue phaedrum, eos nonumy phaedrum ut, soluta interpretaris ad nam. Sed tation sensibus constituam te. Vel altera legimus no, sit vide modus neglegentur ad, ocurreret laboramus disputando ad eum. Laoreet convenire ei vis. At sed agam mollis blandit, ex noster facete ius. Nobis denique vix ei. Ea sumo invenire per, tempor integre an usu, at soluta nostrud signiferumque his. Ex feugait quaestio vel, nonumy prompta ullamcorper vel in. Ea rebum posse constituto quo. Ex nostro malorum eleifend vel. Etiam verterem splendide vel ut, his no tantas commune. Sea cu solet detracto, mei propriae neglegentur eu. Cum ad quas singulis iudicabit, erat adolescens id qui, mel in quem sadipscing. Eu duo eius neglegentur, vix debet mediocrem in, id graece sensibus est. Ex sea veniam omnium veritus, an mea scaevola efficiendi. Duo minim maluisset te, ne qui democritum sadipscing. Eu rebum voluptaria ullamcorper quo. Ei est verterem imperdiet, his delicata vituperata te. Ei utinam insolens temporibus duo, et vis ancillae voluptaria. His clita doctus minimum at. Usu no mutat timeam assueverit, nobis mnesarchum sadipscing at cum. An illud minim nec, no errem dicunt accusamus pro, ad sanctus docendi delicata mel.";

		block->write(str, getFont());

		block->writePageBreak(10.0f);

		block->write("a\t\ta\t\ta\na\ta\ta\ta\na a\ta a\ta a\ta a\na a a\ta a a\ta a a\ta a a\na a a a\ta a a a\ta a a a\ta a a a", getFont(), sf::Color(0xFF), 8);

		block->write("Regular\n", getFont(), sf::Color(0xFF), 15, ui::TextStyle::Regular);
		block->write("Italic\n", getFont(), sf::Color(0xFF), 15, ui::TextStyle::Italic);
		block->write("Underlined\n", getFont(), sf::Color(0xFF), 15, ui::TextStyle::Underlined);
		block->write("Bold\n", getFont(), sf::Color(0xFF), 15, ui::TextStyle::Bold);
		block->write("Strikethrough\n", getFont(), sf::Color(0xFF), 15, ui::TextStyle::StrikeThrough);


		auto blob = block->add<ui::RightFloatingElement>();
		blob->setBackgroundColor(sf::Color(0xBBBBBBFF));

		auto scrollpanel = block->add<ui::ScrollPanel>(true, true, true);
		scrollpanel->setMargin(10.0f);
		//scrollpanel->setHeight(100.0f, true);
		scrollpanel->setBorderRadius(10.0f);
		scrollpanel->setBorderThickness(1.0f);
		scrollpanel->setBorderColor(sf::Color(0xFF));

		//auto img = scrollpanel->inner()->add<ui::Image>("C:/Users/Tim/Pictures/Aye/Jheronimus_Bosch_023.jpg");
		//img->setSize(img->size() * 0.1f, true);

		auto head = scrollpanel->inner()->add<ui::BlockElement>();
		head->setContentAlign(ui::ContentAlign::Center);
		head->write("Robert Huwe", getFont(), sf::Color(0xFF), 20, ui::TextStyle::Underlined);

		auto body = scrollpanel->inner()->add<ui::BlockElement>();
		body->setContentAlign(ui::ContentAlign::Justify);
		body->write(L"So Robert huwe was rocking to some stiffiling music when his cousin came home. “ damn it huwe I told you to stop stuffing the chicken when it is still alive.” “Fuck whistle” Huwe said as he ran down stairs with the cast iron shovel. “ I got this” he said rapidity hitting the chicken on the bonk for a good solid chunk of his life till its dead steaming body lay deceased on the broken crusted floor boards. “is that good enough” Huwe asked. “No fucking damn it huwe its to dead.” His cousin said pick’n up the shovel to continue the beating. “well I’ll get the potatoes then” he said as he walked to the store. As he arrived the typical customers had all run dry leaving only him and his sweet appetite in the coffee shop. “Could I have some potatoes” he asked the nice cranky woman running the cash machine. “no” she said. “ok then” he said walking to trade his appetite for some other limited time appetizer. He came back to his house in one hell of a mood the yack fries were out the camel’s toes was gone and all the loons were fresh out of stock. As he strutted in to the house with a passion he found his cousin lying dead asleep on the floor. He looks around to see that the chicken was still clucking in the microwave.  “That chicken was one stubborn goose” he said putting tinfoil in there with it. After the lighting stopped he removed it and started eating it without pause and days and a night passed before he asked himself. “Where are the potatoes?” He stormed out into the crunchy highway degenerated by the lack of anger in his heart. After he arrived in his local coffee shop he was alone with not even the cranky widow to accompany him. “Could I have a potato.” He asked. “No” he said to himself. He walked over the gently sleeping body at his feet to return home. “Well it beats me.” Said huwe. As the flap they used as a door flapped violently in the wind huwe returned to the tent he called home to celebrate his birthday in two and a half months. “Now that’s what I call a party.” huwe said enthusiastically. As the last rain drop of the sea rain fell for the season. He realized what a simpleton he had been he had forgot the potatoes. “well it doesn’t matter anymore”  he stated.\n", getFont(), sf::Color(0xFF), 15);
		auto foot = scrollpanel->inner()->add<ui::BlockElement>();
		foot->setContentAlign(ui::ContentAlign::Center);
		foot->write("Trevor Hassell", getFont(), sf::Color(0xFF), 15);
	}*/

	ui::run();

	return 0;
}