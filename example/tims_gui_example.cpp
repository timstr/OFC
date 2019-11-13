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

std::unique_ptr<ui::PullDownMenu<ui::String>> makeBasicPullDown(){
    return std::make_unique<ui::PullDownMenu<ui::String>>(
        std::vector<ui::String>{"Bread", "Butter", "Apricots", "Orphans", "Oregonians", "Orifices", "Mankind"},
        getFont(),
        [](const ui::String& s) {
            std::cout << "You chose \"" << s.toAnsiString() << "\"\n";
		}
    );
}

std::unique_ptr<ui::PullDownMenu<ui::Color>> makeRedPullDown(){
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
	auto pdptr = std::make_unique<ui::PullDownMenu<ui::Color>>(
        color_options,
        getFont()
    );

    auto& pd = *pdptr;

    pd.setOnChange([&pd](const ui::Color& c){
        pd.setNormalColor(c);
        pd.setHoverColor(ui::interpolate(c, 0xFFFFFFFF, 0.5f));
        pd.setActiveColor(ui::interpolate(c, 0x000000FF, 0.3f));
    });

    return pdptr;
}

class BoxContainer : public ui::FreeContainer, public ui::BoxElement {
private:
    void render(sf::RenderWindow& rw) override {
        ui::BoxElement::render(rw);
        ui::FreeContainer::render(rw);
    }
};


std::unique_ptr<ui::Element> makeExampleProcessor(){
    class Knob : public ui::Control, public ui::BoxElement {
    public:
        Knob(){
            setSize({30.0f, 30.0f}, true);
            setBackgroundColor(0x22CC44FF);
            setBorderColor(0x80);
            setBorderRadius(15.0f);
            setBorderThickness(2.0f);
        }
    private:
        void onMouseOver() override {
            setBackgroundColor(0x116622FF);
        }

        void onMouseOut() override {
            setBackgroundColor(0x22CC44FF);
        }
    };

    class DragButton : public ui::Control, public ui::BoxElement {
    public:
        DragButton(ui::Draggable& parent) : m_parent(parent) {
            setBackgroundColor(0x0088AAFF);
            setSize({30.0, 30.0f}, true);
        }

    private:
        bool onLeftClick(int) override {
            m_parent.startDrag();
            return true;
        }

        void onLeftRelease() override {
            m_parent.stopDrag();
        }

        ui::Draggable& m_parent;
    };

    class DraggableGrid : public ui::GridContainer, public ui::Draggable {
    public:
        using ui::GridContainer::GridContainer;
    };

    auto mainCont = std::make_unique<DraggableGrid>(3, 2);
    mainCont->setShrink(true);

    auto& left = mainCont->putCell<ui::VerticalList>(0, 1);
    left.push_back<Knob>();
    left.push_back<Knob>();
    left.push_back<Knob>();

    auto& topCont = mainCont->putCell<ui::FreeContainer>(1, 0);
    auto& top = topCont.add<ui::HorizontalList>(
        ui::FreeContainer::InsideRight,
        ui::FreeContainer::Center
    );

    top.push_back<Knob>();
    top.push_back<Knob>();

    auto& corner = mainCont->putCell<DragButton>(
        0,
        0,
        *mainCont
    );

    auto& right = mainCont->putCell<ui::VerticalList>(2, 1);

    right.push_back<Knob>();

    auto& body = mainCont->putCell<BoxContainer>(1, 1);
    body.setBackgroundColor(0x008800FF);
    auto& content = body.add<ui::VerticalList>(
        ui::FreeContainer::Center,
        ui::FreeContainer::Center
    );
    content.setPadding(5.0f);
    content.push_back<ui::Text>(
        "Example Processor",
        getFont()
    );
    content.push_back<ui::CallbackButton>(
        "Add Output",
        getFont(),
        [&](){
            auto& fc = right.push_back<ui::Boxed<ui::FreeContainer>>();
            fc.setBackgroundColor(0x4040FFFF);
            fc.setBorderRadius(10.0f);
            fc.setBorderColor(0xFF);
            fc.setBorderThickness(1.0f);
            fc.setMinWidth(75.0f);
            fc.add<Knob>(ui::FreeContainer::InsideRight, ui::FreeContainer::Center);
            fc.add<ui::CallbackButton>(
                ui::FreeContainer::InsideLeft,
                ui::FreeContainer::Center,
                "x",
                getFont(),
                [&](){
                    fc.close();
                }
            );
            //right.push_back<Knob>();
        }
    );

    return mainCont;
}

std::unique_ptr<ui::Element> makeRandomControl(){
    const auto dist = std::uniform_int_distribution<int>{0, 9};

    switch (dist(randeng)){
    case 0:
        return std::make_unique<ui::ToggleButton>(
            false,
            getFont(),
            std::function<void(bool)>{},
            std::pair<ui::String, ui::String>{"bada bing", "bada boooooom"}
        );
    case 1:
        return std::make_unique<ui::CallbackButton>(
            "Bang",
            getFont(),
            [](){
                std::cout << "Bang!\n";
            }
        );
    case 2:
        return std::make_unique<ui::TextEntry>(
            getFont()
        );
    case 3:
        return std::make_unique<ui::CallbackTextEntry>(
            getFont(),
            [](const ui::String& s){
                std::cout << "You entered: \"" + s.toAnsiString() + "\"\n";
            },
            [](const ui::String& s){
                return std::find(s.begin(), s.end(), ' ') == s.end();
            }
        );
    case 4:
        return makeBasicPullDown();
    case 5:
        return makeRedPullDown();
    case 6:
        return std::make_unique<ui::Slider<float>>(
            5.0f,
            0.0f,
            10.0f,
            getFont(),
            [](float v){
                std::cout << "The value is now " << v << '\n';
            }
        );
    case 7:
        return std::make_unique<ui::Slider<int>>(
            50,
            0,
            100,
            getFont(),
            [](int v){
                std::cout << "The value is now " << v << '\n';
            }
        );
    case 8:
        return makeExampleProcessor();
    case 9:
        return std::make_unique<ui::NumberTextEntry<double>>(
            0.0,
            0.0,
            1.0,
            getFont(),
            [](double v){
                std::cout << "You entered " << v << '\n';
            }
        );
    /*case 10:
        {
            auto sp = std::make_unique<ui::ScrollPanel<ui::FlowContainer>>(true, true, true);
            sp->inner().add<ui::Image>("C:/Users/Tim/Pictures/untitled.png");
            return sp;
        }*/
    }
    throw std::runtime_error("Aaaarg");
}

std::unique_ptr<ui::GridContainer> makeGridLayout(){
    auto cont = std::make_unique<ui::GridContainer>(3, 3);
    /*cont->setRowHeight(0, 1.0f);
    cont->setRowHeight(1, 2.0f);
    cont->setRowHeight(2, 4.0f);
    cont->setColumnWidth(0, 5.0f);
    cont->setColumnWidth(1, 1.0f);
    cont->setColumnWidth(2, 1.0f);*/

    const auto add_cell = [&cont](size_t x, size_t y, ui::Color color){
        auto& c = cont->putCell<BoxContainer>(x, y);
        c.setBackgroundColor(color);
        c.setClipping(true);

        c.adopt(ui::FreeContainer::Center, ui::FreeContainer::Center, makeRandomControl());
        //c.adopt(makeRandomControl());
    };
    
    add_cell(0, 0, 0xBBBBBBFF);
    add_cell(1, 0, 0xDDDDDDFF);
    add_cell(2, 0, 0xBBBBBBFF);
    add_cell(0, 1, 0xDDDDDDFF);
    add_cell(1, 1, 0xBBBBBBFF);
    add_cell(2, 1, 0xDDDDDDFF);
    add_cell(0, 2, 0xBBBBBBFF);
    add_cell(1, 2, 0xDDDDDDFF);
    add_cell(2, 2, 0xBBBBBBFF);

    return cont;
}

std::unique_ptr<ui::FlowContainer> makeFlowLayout(){
    auto cont = std::make_unique<ui::FlowContainer>();

    cont->write("a b c d e f g h i j k l m n o p q r s t u v A B C D E F G H I J K L M N O P Q R S T U V W X Y Z", getFont());

    return cont;
}

int main(){
    
    ui::Window& win = ui::Window::create(1000, 800, "Tim's GUI Test");

    //auto& root = win.setRoot<ui::FreeContainer>();

    auto gcp = makeGridLayout();
    auto& gc = *gcp;

    //root.adopt(std::move(gcp));
    win.setRoot(std::move(gcp));

    //gc.setMinSize({200.0f, 200.0f});
    gc.setPos({50.0f, 50.0f});

    //gc.putCell(2, 0, makeFlowLayout());
    gc.putCell(2, 2, makeGridLayout());

    //makeFlowLayout(win);

    //auto& root = win.setRoot<ui::FreeContainer>();
    //root.add<DragButton>();
    
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