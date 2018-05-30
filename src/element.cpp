#include "gui/element.h"
#include "gui/gui.h"
#include <set>

namespace ui {
	namespace {
		const float epsilon = 0.0001f;
	}

	Element::Element(DisplayStyle _display_style) :
		display_style(_display_style),
		pos({0.0f, 0.0f}),
		size({100.0f, 100.0f}),
		min_size({0.0f, 0.0f}),
		max_size({10000.0f, 10000.f}),
		old_total_size({0.0f, 0.0f}),
		disabled(false),
		visible(true),
		clipping(false),
		dirty(true),
		layout_index(0),
		padding(10.0f),
		margin(5.0f) {

	}

	void Element::disable(){
		disabled = true;
	}

	void Element::enable(){
		disabled = false;
	}

	bool Element::isEnabled() const {
		return !disabled;
	}

	void Element::setVisible(bool is_visible){
		if (visible != is_visible && display_style != DisplayStyle::Free){
			makeDirty();
		}
		visible = is_visible;
	}

	bool Element::isVisible() const {
		return visible;
	}

	void Element::setClipping(bool _clipping){
		clipping = _clipping;
	}
	
	vec2 Element::getPos() const {
		return pos;
	}
	
	void Element::setPos(vec2 _pos){
		if (abs(pos.x - _pos.x) + abs(pos.y - _pos.y) > epsilon){
			pos = _pos;
			// makeDirty();
		}
	}
	
	vec2 Element::getSize() const {
		return size;
	}
	
	void Element::setSize(vec2 _size, bool force){
		_size = vec2(std::max(_size.x, 0.0f), std::max(_size.y, 0.0f));
		if (abs(size.x - _size.x) + abs(size.y - _size.y) > epsilon){
			size = _size;
			makeDirty();
		}
		if (force){
			min_size = _size;
			max_size = _size;
			makeDirty();
		}
	}
	
	void Element::setMinSize(vec2 _min_size){
		_min_size = vec2(std::max(_min_size.x, 0.0f), std::max(_min_size.y, 0.0f));
		min_size = _min_size;
		makeDirty();
	}

	void Element::setMaxSize(vec2 _max_size){
		_max_size = vec2(std::max(_max_size.x, 0.0f), std::max(_max_size.y, 0.0f));
		max_size = _max_size;
		makeDirty();
	}
	
	Element::~Element(){
		
	}
	
	void Element::close(){
		onClose();
		while (!children.empty()){
			if (children.back()->inFocus()){
				grabFocus();
			}
			children.pop_back();
		}
		if (auto p = parent.lock()){
			p->remove(shared_from_this());
		}
	}

	void Element::onClose(){

	}
	
	bool Element::hit(vec2 testpos) const {
		return ((testpos.x >= 0.0f) && (testpos.x < size.x) && (testpos.y >= 0.0f) && (testpos.y < size.y));
	}
	
	vec2 Element::localMousePos() const {
		vec2 pos = (vec2)sf::Mouse::getPosition(getContext().getRenderWindow());
		std::shared_ptr<const Element> element = shared_from_this();
		while (element){
			pos -= element->pos;
			element = element->parent.lock();
		}
		return pos;
	}
	
	vec2 Element::rootPos() const {
		vec2 pos = {0, 0};
		std::shared_ptr<const Element> element = shared_from_this();
		while (element){
			pos += element->pos;
			element = element->parent.lock();
		}
		return pos;
	}
	
	bool Element::onLeftClick(int clicks){
		return false;
	}
	
	bool Element::onLeftRelease(){
		return false;
	}
	
	bool Element::onRightClick(int clicks){
		return false;
	}
	
	bool Element::onRightRelease(){
		return false;
	}
	
	bool Element::leftMouseDown() const {
		if (inFocus()){
			return sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
		} else {
			return false;
		}
	}
	
	bool Element::rightMouseDown() const {
		if (inFocus()){
			return sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
		} else {
			return false;
		}
	}
	
	bool Element::onScroll(float delta_x, float delta_y){
		return false;
	}
	
	void Element::startDrag(){
		if (display_style == DisplayStyle::Free){
			grabFocus();
			getContext().setDraggingElement(shared_from_this(), (vec2)sf::Mouse::getPosition(getContext().getRenderWindow()) - pos);
		}
	}
	
	void Element::onDrag(){
		
	}
	
	void Element::stopDrag(){
		if (dragging()){
			getContext().setDraggingElement({});
		}
	}
	
	bool Element::dragging() const {
		return (getContext().getDraggingElement() == shared_from_this());
	}
	
	bool Element::onHover(){
		return false;
	}
	
	bool Element::onHoverWith(std::shared_ptr<Element> element){
		return false;
	}
	
	void Element::drop(vec2 local_pos){
		vec2 pos = rootPos() + local_pos;
		if (auto element = root().findElementAt(pos, shared_from_this())){
			auto self = shared_from_this();
			do {
				if (element->onDrop(self)){
					return;
				}
			} while (element = element->parent.lock());
		}
	}
	
	bool Element::onDrop(std::shared_ptr<Element> element){
		return false;
	}
	
	void Element::onFocus(){

	}
	
	bool Element::inFocus() const {
		return (getContext().getCurrentElement() == shared_from_this());
	}
	
	void Element::onLoseFocus(){

	}
	
	void Element::grabFocus(){
		getContext().focusTo(shared_from_this());
	}
	
	bool Element::onKeyDown(Key key){
		return false;
	}
	
	bool Element::onKeyUp(Key key){
		return false;
	}
	
	bool Element::keyDown(Key key) const {
		return inFocus() && sf::Keyboard::isKeyPressed(key);
	}
	
	void Element::remove(std::shared_ptr<Element> element){
		if (element){
			for (auto it = children.begin(); it != children.end(); ++it){
				if (*it == element){
					if (element->inFocus()){
						grabFocus();
					}
					children.erase(it);
					element->parent = {};
					organizeLayoutIndices();
					makeDirty();
					return;
				}
			}
		}
	}
	
	std::shared_ptr<Element> Element::release(std::shared_ptr<Element> element){
		if (element){
			for (auto it = children.begin(); it != children.end(); ++it){
				if (*it == element){
					children.erase(it);
					element->parent = {};
					makeDirty();
					return element;
				}
			}
		}
		return nullptr;
	}
	
	void Element::bringToFront(){
		if (auto p = parent.lock()){
			auto self = shared_from_this();
			for (auto it = p->children.begin(); it != p->children.end(); ++it){
				if (*it == self){
					p->children.erase(it);
					p->children.push_back(self);
					return;
				}
			}
		}
	}
	
	void Element::clear(){
		children.clear();
		makeDirty();
	}
	
	std::shared_ptr<Element> Element::findElementAt(vec2 _pos, std::shared_ptr<Element> exclude){
		if (!visible || disabled){
			return nullptr;
		}

		if (clipping && ((_pos.x < 0.0f) || (_pos.x > size.x) || (_pos.y < 0.0) || (_pos.y > size.y))){
			return nullptr;
		}

		if (exclude == shared_from_this()){
			return nullptr;
		}

		std::shared_ptr<Element> element;
		for (auto it = children.rbegin(); it != children.rend(); ++it){
			element = (*it)->findElementAt(_pos - (*it)->pos, exclude);
			if (element){
				return element;
			}
		}

		if (this->hit(_pos)){
			return shared_from_this();
		}

		return nullptr;
	}
	
	void Element::render(sf::RenderWindow& rw){
		sf::RectangleShape rect;
		rect.setOutlineColor(sf::Color(0xFF));
		rect.setOutlineThickness(1.0f);
		rect.setSize(getSize() - vec2(2.0f, 2.0f));
		rect.setPosition({1.0f, 1.0f});
		rect.setFillColor(sf::Color(0xFFFFFFFF));
		rw.draw(rect);
	}
	
	void Element::renderChildren(sf::RenderWindow& renderwindow){
		for (auto it = children.begin(); it != children.end(); ++it){
			const std::shared_ptr<Element>& child = *it;
			if (child->visible){
				if (child->clipping){
					getContext().translateView(child->pos);
					sf::FloatRect rect = getContext().getClipRect();
					vec2 pos = getContext().getViewOffset();
					getContext().intersectClipRect(sf::FloatRect(-pos, child->size));
					getContext().updateView();
					child->render(renderwindow);
					child->renderChildren(renderwindow);
					getContext().setClipRect(rect);
					getContext().translateView(-child->pos);
					getContext().updateView();
				} else {
					getContext().translateView(child->pos);
					getContext().updateView();
					child->render(renderwindow);
					child->renderChildren(renderwindow);
					getContext().translateView(-child->pos);
					getContext().updateView();
				}
			}
		}
	}
	
	int Element::getNextLayoutIndex() const {
		int max = 0;
		for (const auto& child : children){
			max = std::max(child->layout_index, max);
		}
		return max + 1;
	}
		
	void Element::organizeLayoutIndices(){
		std::set<std::pair<int, std::shared_ptr<Element>>> index_set;
		for (const auto& child : children){
			index_set.insert({child->layout_index, child});
		}
		int i = 0;
		for (const auto& mapping : index_set){
			mapping.second->layout_index = i;
			++i;
		}
	}
	
	const std::vector<std::shared_ptr<Element>>& Element::getChildren() const {
		return children;
	}
	
	std::weak_ptr<Element> Element::getParent() const {
		return parent;
	}

	void Element::setPadding(float _padding){
		if (abs(padding - _padding) > epsilon){
			padding = std::max(_padding, 0.0f);
			makeDirty();
		}
	}

	float Element::getPadding() const {
		return padding;
	}

	void Element::setMargin(float _margin){
		_margin = std::max(_margin, 0.0f);
		if (abs(margin - _margin) > epsilon){
			margin = _margin;
			makeDirty();
		}
	}

	float Element::getMargin() const {
		return margin;
	}

	void Element::adopt(std::shared_ptr<Element> child){
		children.push_back(child);
		child->parent = weak_from_this();
		child->layout_index = getNextLayoutIndex();
		organizeLayoutIndices();
		makeDirty();
	}

	void Element::makeDirty(){
		dirty = true;
	}

	bool Element::isDirty() const {
		return dirty;
	}

	void Element::makeClean(){
		dirty = false;
	}

	bool Element::update(float width_avail){
		width_avail = std::min(std::max(width_avail, min_size.x), max_size.x);

		if (display_style == DisplayStyle::Block){
			setSize({
				width_avail,
				size.y
			});
		}

		if (!isDirty()){
			for (auto child = children.begin(); !isDirty() && child != children.end(); ++child){
				if ((*child)->update((*child)->size.x)){
					makeDirty();
				}
			}
			if (!isDirty()){
				return false;
			}
		}

		// at this point, this element is dirty
		makeClean();

		// calculate own width and arrange children
		if (display_style == DisplayStyle::Free){
			vec2 contentsize = arrangeChildren(size.x);
			size = vec2(
				std::min(std::max({size.x, contentsize.x, min_size.x}), max_size.x),
				std::min(std::max({size.y, contentsize.y, min_size.y}), max_size.y)
			);
			return false;
		} else {
			vec2 newsize = arrangeChildren(width_avail);
			if (display_style == DisplayStyle::Block){
				newsize.x = std::max(width_avail, newsize.x);
			}
			size = vec2(
				std::min(std::max(newsize.x, min_size.x), max_size.x),
				std::min(std::max(newsize.y, min_size.y), max_size.y)
			);
			if (size.x > width_avail){
				arrangeChildren(size.x);
			}
			vec2 new_total_size = size + vec2(2.0f * margin, 2.0f * margin);
			float diff = abs(old_total_size.x - new_total_size.x) + abs(old_total_size.y - new_total_size.y);
			old_total_size = new_total_size;
			return diff > epsilon;
		}
	}

	vec2 Element::arrangeChildren(float width_avail){
		vec2 contentsize = {0, 0};
		float xpos = padding;
		float ypos = padding;
		float next_ypos = ypos;

		std::vector<std::shared_ptr<Element>> sorted = children;

		auto comp = [](const std::shared_ptr<Element>& l, const std::shared_ptr<Element>& r){
			return l->layout_index < r->layout_index;
		};
		std::sort(sorted.begin(), sorted.end(), comp);

		for (const auto& element : sorted){
			if (!element->isVisible()){
				continue;
			}
			switch (element->display_style){
				case DisplayStyle::Block:
					// block elements appear on a new line and may take up the full
					// width available and as much height as needed
					{
						xpos = padding;
						ypos = next_ypos + element->margin;
						element->setPos({xpos + element->margin, ypos});
						float avail = width_avail - 2.0f * (padding + element->margin);
						element->update(avail);
						ypos = next_ypos + element->getSize().y + 2.0f * element->margin;
						next_ypos = ypos;
					}
					break;
				case DisplayStyle::Inline:
					// inline elements appear inline and will take up only as much space as needed
					// inline-block elements appear inline but may take up any desired amount of space
					{
						element->setPos({xpos + element->margin, ypos + element->margin});
						float avail = width_avail - (element->margin + padding + xpos);
						element->update(avail);
						// if the element exceeds the end of the line, put it on a new line and rearrange
						if (xpos + element->getSize().x + 2.0f * element->margin + padding > width_avail){
							xpos = padding;
							ypos = next_ypos;
							avail = width_avail - 2.0f * (padding + element->margin);
							element->setPos({xpos + element->margin, ypos + element->margin});
							element->update(avail);
						}
						xpos += element->size.x + 2.0f * element->margin;
						next_ypos = std::max(next_ypos, ypos + element->getSize().y + 2.0f * element->margin);
					}
					break;
				case DisplayStyle::Free:
					// free elements do not appear as flow elements but are positioned
					// relative to their parent at their x/y position (like the classic ui)
					
					element->update(element->size.x);
					break;
			}
			if (element->display_style != DisplayStyle::Free){
				contentsize = vec2(
					std::max(contentsize.x, element->pos.x + element->size.x + element->margin + padding),
					std::max(contentsize.y, element->pos.y + element->size.y + element->margin + padding)
				);
			}
		}

		return contentsize;
	}

	FreeElement::FreeElement() : Element(DisplayStyle::Free) {

	}

	InlineElement::InlineElement() : Element(DisplayStyle::Inline) {

	}

	BlockElement::BlockElement() : Element(DisplayStyle::Block) {

	}

}