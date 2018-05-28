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
	
	void Element::setSize(vec2 _size){
		_size = vec2(std::max(_size.x, 0.0f), std::max(_size.y, 0.0f));
		if (abs(size.x - _size.x) + abs(size.y - _size.y) > epsilon){
			size = _size;
			makeDirty();
		}
	}
	
	void Element::setMinSize(vec2 _min_size){
		_min_size = vec2(std::max(_min_size.x, 0.0f), std::max(_min_size.y, 0.0f));
		min_size = _min_size;
		makeDirty();
	}
	
	Element::~Element(){
		while (!children.empty()){
			if (children.back()->inFocus()){
				grabFocus();
			}
			children.pop_back();
		}
	}
	
	void Element::close(){
		if (auto p = parent.lock()){
			p->remove(weak_from_this());
		}
		onClose();
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
	
	void Element::onLeftClick(int clicks){

	}
	
	void Element::onLeftRelease(){

	}
	
	void Element::onRightClick(int clicks){

	}
	
	void Element::onRightRelease(){

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
	
	void Element::onScroll(float delta_x, float delta_y){

	}
	
	void Element::startDrag(){
		if (display_style == DisplayStyle::Free){
			grabFocus();
			getContext().setDraggingElement(weak_from_this(), (vec2)sf::Mouse::getPosition(getContext().getRenderWindow()) - pos);
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
		return (getContext().getDraggingElement().lock() == shared_from_this());
	}
	
	void Element::onHover(){

	}
	
	void Element::onHoverWith(std::weak_ptr<Element> element){

	}
	
	void Element::drop(vec2 local_pos){
		vec2 pos = rootPos() + local_pos;
		if (auto element = root().findElementAt(pos, weak_from_this()).lock()){
			auto self = weak_from_this();
			do {
				if (element->onDrop(self)){
					return;
				}
			} while (element = element->parent.lock());
		}
	}
	
	bool Element::onDrop(std::weak_ptr<Element> element){
		return false;
	}
	
	void Element::onFocus(){

	}
	
	bool Element::inFocus() const {
		return (getContext().getCurrentElement().lock() == shared_from_this());
	}
	
	void Element::onLoseFocus(){

	}
	
	void Element::grabFocus(){
		getContext().focusTo(weak_from_this());
	}
	
	void Element::onKeyDown(Key key){

	}
	
	void Element::onKeyUp(Key key){

	}
	
	bool Element::keyDown(Key key) const {
		return inFocus() && sf::Keyboard::isKeyPressed(key);
	}
	
	void Element::remove(std::weak_ptr<Element> element){
		if (auto elem = element.lock()){
			for (auto it = children.begin(); it != children.end(); ++it){
				if (*it == elem){
					if ((*it)->inFocus()){
						grabFocus();
					}
					children.erase(it);
					organizeLayoutIndices();
					makeDirty();
					return;
				}
			}
		}
	}
	
	std::shared_ptr<Element> Element::release(std::weak_ptr<Element> element){
		if (auto elem = element.lock()){
			for (auto it = children.begin(); it != children.end(); ++it){
				if (*it == elem){
					std::shared_ptr<Element> child = *it;
					children.erase(it);
					makeDirty();
					return child;
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
	
	std::weak_ptr<Element> Element::findElementAt(vec2 _pos, std::weak_ptr<Element> exclude){
		if (!visible || disabled){
			return {};
		}

		if (clipping && ((_pos.x < 0.0f) || (_pos.x > size.x) || (_pos.y < 0.0) || (_pos.y > size.y))){
			return {};
		}

		if (exclude.lock() == shared_from_this()){
			return {};
		}

		std::weak_ptr<Element> element;
		for (auto it = children.rbegin(); it != children.rend(); ++it){
			element = (*it)->findElementAt(_pos - (*it)->pos, exclude);
			if (auto elem = element.lock()){
				return elem;
			}
		}

		if (this->hit(_pos)){
			return weak_from_this();
		}

		return {};
	}
	
	void Element::render(sf::RenderWindow& rw){

		sf::RectangleShape rect;
		rect.setOutlineColor(sf::Color(0xFF));
		rect.setOutlineThickness(1);

		// margin
		rect.setPosition({-margin + 1.0f, -margin + 1.0f});
		rect.setSize(size + vec2(2.0f * margin - 2.0f, 2.0f * margin - 2.0f));
		rect.setFillColor(sf::Color(0xf9cc9dff));
		if (rect.getSize().x > 0 && rect.getSize().y > 0){
			rw.draw(rect);
		}

		// padding
		rect.setPosition({1.0f, 1.0f});
		rect.setSize(size - vec2(2.0f, 2.0f));
		rect.setFillColor(sf::Color(0xc3d08bff));
		if (rect.getSize().x > 0 && rect.getSize().y > 0){
			rw.draw(rect);
		}

		// content
		rect.setPosition({padding + 1.0f, padding + 1.0f});
		rect.setSize(size - vec2(2.0f * padding + 2.0f, 2.0f * padding + 2.0f));
		//rect.setFillColor(sf::Color((((uint32_t)std::hash<Element*>{}(this)) & 0xFFFFFF00) | 0x80));
		rect.setFillColor(sf::Color(0x8cb6c0ff));
		if (rect.getSize().x > 0 && rect.getSize().y > 0){
			rw.draw(rect);
		}
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
	
	std::vector<std::weak_ptr<Element>> Element::getChildren() const {
		std::vector<std::weak_ptr<Element>> ret;
		ret.reserve(children.size());
		for (auto it = children.begin(); it != children.end(); ++it){
			ret.push_back(*it);
		}
		return ret;
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
		width_avail = std::max(width_avail, min_size.x);

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
			arrangeChildren(size.x);
			return false;
		} else {
			vec2 oldsize = size;
			vec2 newsize = arrangeChildren(width_avail);
			if (display_style == DisplayStyle::Block){
				newsize.x = std::max(width_avail, newsize.x);
			}
			size = vec2(
				std::max(newsize.x, min_size.x),
				std::max(newsize.y, min_size.y)
			);
			float diff = abs(newsize.x - oldsize.x) + abs(newsize.y - oldsize.y);
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
						if (xpos + element->getSize().x + element->margin + padding > width_avail){
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
					std::max(contentsize.x, element->pos.x + element->size.x + padding),
					std::max(contentsize.y, element->pos.y + element->size.y + padding)
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