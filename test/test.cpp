#include "gui/gui.h"
#include "gui/helpers.h"

#include "gui/roundrectangle.h"

#include <iostream>
#include <random>
#include <sstream>

#include "fontspath.h"

std::random_device randdev;
std::mt19937 randeng { randdev() };

sf::Font& getFont() {
	static sf::Font font;
	static bool loaded;
	if (!loaded) {
		font.loadFromFile(fonts_path + "/JosefinSans-Bold.ttf");
		loaded = true;
	}
	return font;
}

struct TestElement : ui::FreeElement {
	TestElement(std::string name) : name(name) {
		std::cout << name << " was constructed" << std::endl;
		changeColor();
		label = add<ui::Text>(name, getFont());
		add<ui::CallbackButton>("Change Colour", getFont(), [this] {
			changeColor();
		});
		add<ui::PullDownMenu<std::string>>(std::vector<std::string>{"Bread", "Butter", "Apricots", "Orphans", "Oregonians", "Orifices", "Mankind"},
										   getFont(),
										   [this](const std::string& s) {
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
		add<ui::PullDownMenu<sf::Color>>(color_options, getFont(), [this](const sf::Color& color) {
			this->setBackgroundColor(color);
		});

		add<ui::TextEntryHelper>("", getFont(), [this](const std::wstring& text) {
			this->label->setText(text);
		});

		float val = 0.5f;
		float min = 0.0f;
		float max = 1.0f;
		auto numbrocont = add<ui::InlineElement>();
		numbrocont->setSize({ 100.0f, 35.0f }, true);
		auto numbro = numbrocont->add<ui::Text>(std::to_string(val), getFont());
		auto onChange = [numbro](float v) {
			numbro->setText(std::to_string(v));
		};
		add<ui::NumberTextEntry<float>>(val, min, max, getFont(), onChange);
		add<ui::Slider>(val, min, max, getFont(), onChange);

		add<ui::CallbackButton>("add something", getFont(), [this] {
			write("blop", getFont());
		});

		add<SizeButton>(*this);
	}
	~TestElement() {
		std::cout << name << " was destroyed" << std::endl;
	}

	void onClose() override {
		std::cout << name << " was closed" << std::endl;
	}

	void changeColor() {
		std::uniform_int_distribution<unsigned> dist(0, 0xFF);
		setBackgroundColor(sf::Color(dist(randeng),
			dist(randeng),
			dist(randeng),
			255));
	}

	bool onLeftClick(int clicks) override {
		bringToFront();
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
			auto self = shared_from_this();
			vec2 oldsize = getSize();
			vec2 newsize = { (float)sdist(randeng), (float)sdist(randeng) };
			float oldmargin = getMargin();
			float newmargin = (float)mdist(randeng);
			float oldpadding = getPadding();
			float newpadding = (float)mdist(randeng);
			ui::startTransition(1.0, [=](float t) {
				float x = 0.5f - 0.5f * cos(t * 3.141592654f);
				self->setSize(oldsize * (1.0f - x) + newsize * x);
				self->setMargin(oldmargin * (1.0f - x) + newmargin * x);
				self->setPadding(oldpadding * (1.0f - x) + newpadding * x);
				self->setBorderRadius(oldpadding * (1.0f - x) + newpadding * x);
			});
		} else if (clicks == 2) {
			std::cout << name << " was right-clicked twice" << std::endl;
		}
		return true;
	}

	void onRightRelease() override {
		std::cout << name << " was right released" << std::endl;
	}

	void onFocus() override {
		bringToFront();
		std::cout << name << " gained focus" << std::endl;
	}

	void onLoseFocus() override {
		std::cout << name << " lost focus" << std::endl;
	}

	bool onKeyDown(ui::Key key) override {
		std::cout << name << " - [" << key << "] was pressed" << std::endl;
		if (key == ui::Key::Escape) {
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
		if (element) {
			if (auto w = std::dynamic_pointer_cast<TestElement, Element>(element)) {
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

	const std::string name;
	std::shared_ptr<ui::Text> label;

	struct SizeButton : ui::FreeElement {
		SizeButton(TestElement& _parent) : parent(_parent) {
			setSize({ 20.0f, 20.0f });
			setBackgroundColor(sf::Color(0x808080FF));
			setBorderThickness(0.0f);
			setXPositionStyle(ui::PositionStyle::InsideRight);
			setYPositionStyle(ui::PositionStyle::InsideBottom);
		}

		bool onLeftClick(int clicks) override {
			startDrag();
			return true;
		}

		void onLeftRelease() override {
			stopDrag();
		}

		void onDrag() override {
			parent.setSize(getPos() + getSize());
		}

		TestElement& parent;
	};
};

int main(int argc, char** argcv) {

	ui::init(1000, 800, "Tim's GUI Test", 60);

	{
		auto block = ui::root().add<ui::BlockElement>();
		block->setBackgroundColor(sf::Color(0xFFFFFFFF));

		block->setMinSize({ 0, 100 });

		auto pops = block->add<TestElement>("Pops");
		pops->setLayoutStyle(ui::LayoutStyle::FloatLeft);
		pops->add<TestElement>("Hector");

		block->add<TestElement>("Hank");

		block->setContentAlign(ui::ContentAlign::Justify);

		auto widg = block->add<ui::RightFloatingElement>();
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

		std::wstring str = L"\tLorem ipsum dolor sit amet, ne choro legendos expetendis quo. Ei mel nibh dissentiunt, ius nibh nobis ei, at mel feugiat platonem. Et hinc graeco veritus pro. Liber inimicus repudiare ex usu. Ad nec evertitur sadipscing, id oratio legere nec. Ad eum eros congue phaedrum, eos nonumy phaedrum ut, soluta interpretaris ad nam. Sed tation sensibus constituam te. Vel altera legimus no, sit vide modus neglegentur ad, ocurreret laboramus disputando ad eum. Laoreet convenire ei vis. At sed agam mollis blandit, ex noster facete ius. Nobis denique vix ei. Ea sumo invenire per, tempor integre an usu, at soluta nostrud signiferumque his. Ex feugait quaestio vel, nonumy prompta ullamcorper vel in. Ea rebum posse constituto quo. Ex nostro malorum eleifend vel. Etiam verterem splendide vel ut, his no tantas commune. Sea cu solet detracto, mei propriae neglegentur eu. Cum ad quas singulis iudicabit, erat adolescens id qui, mel in quem sadipscing. Eu duo eius neglegentur, vix debet mediocrem in, id graece sensibus est. Ex sea veniam omnium veritus, an mea scaevola efficiendi. Duo minim maluisset te, ne qui democritum sadipscing. Eu rebum voluptaria ullamcorper quo. Ei est verterem imperdiet, his delicata vituperata te. Ei utinam insolens temporibus duo, et vis ancillae voluptaria. His clita doctus minimum at. Usu no mutat timeam assueverit, nobis mnesarchum sadipscing at cum. An illud minim nec, no errem dicunt accusamus pro, ad sanctus docendi delicata mel.";

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

		auto output = block->add<ui::BlockElement>();
		output->setBackgroundColor(sf::Color(0xFF));
		output->setBorderRadius(10.0f);
		output->setPadding(0.0f);
		output->setMargin(10.0f);
		output->setHeight(100.0f, true);
		output->setClipping(true);

		auto textcont = output->add<ui::FreeElement>();
		textcont->setPadding(0.0f);
		textcont->setMargin(0.0f);
		textcont->setXPositionStyle(ui::PositionStyle::InsideLeft, 5.0f);
		textcont->setYPositionStyle(ui::PositionStyle::InsideBottom, 5.0f);

		auto log = [textcont](const std::string& str) {
			textcont->write(str + "\n", getFont(), sf::Color(0xFFFFFFFF), 12);
		};

		blob->write("float:\n", getFont());
		blob->add<ui::NumberTextEntry<float>>(50.0f, 0.0f, 100.0f, getFont(), [&](float val) {
			log(std::to_string(val));
		});
		blob->writeLineBreak();

		blob->write("double:\n", getFont());
		blob->add<ui::NumberTextEntry<double>>(50.0, 0.0, 100.0, getFont(), [&](double val) {
			log(std::to_string(val));
		});
		blob->writeLineBreak();

		blob->write("int:\n", getFont());
		blob->add<ui::NumberTextEntry<int>>(50, 0, 100, getFont(), [&](int val) {
			log(std::to_string(val));
		});

	}

	ui::run();

	return 0;
}