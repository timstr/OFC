#include "gui/window.h"
#include "gui/gui.h"
#include <set>

namespace ui {
	namespace {
		const float epsilon = 0.0001f;
	}

	Window::Window(DisplayStyle _display_style) :
		display_style(_display_style),
		pos({0.0f, 0.0f}),
		size({100.0f, 100.0f}),
		min_size({0.0f, 0.0f}),
		disabled(false),
		visible(true),
		clipping(false),
		dirty(true) {

	}
	vec2 Window::getPos() const {
		return pos;
	}
	void Window::setPos(vec2 _pos){
		if (abs(pos.x - _pos.x) + abs(pos.y - _pos.y) > epsilon){
			pos = _pos;
			// makeDirty();
		}
	}
	vec2 Window::getSize() const {
		return size;
	}
	void Window::setSize(vec2 _size){
		_size = vec2(std::max(_size.x, 0.0f), std::max(_size.y, 0.0f));;
		if (abs(size.x - _size.x) + abs(size.y != _size.y) > epsilon){
			size = _size;
			// makeDirty();
		}
	}
	void Window::setMinSize(vec2 size){
		min_size = vec2(std::max(size.x, 0.0f), std::max(size.y, 0.0f));
	}
	Window::~Window(){
		while (!childwindows.empty()){
			if (childwindows.back()->inFocus()){
				grabFocus();
			}
			childwindows.pop_back();
		}
	}
	void Window::close(){
		if (auto p = parent.lock()){
			p->remove(weak_from_this());
		}
	}
	bool Window::hit(vec2 testpos) const {
		return ((testpos.x >= 0.0f) && (testpos.x < size.x) && (testpos.y >= 0.0f) && (testpos.y < size.y));
	}
	vec2 Window::localMousePos() const {
		vec2 pos = (vec2)sf::Mouse::getPosition(getContext().getRenderWindow());
		std::shared_ptr<const Window> window = shared_from_this();
		while (window){
			pos -= window->pos;
			window = window->parent.lock();
		}
		return pos;
	}
	vec2 Window::rootPos() const {
		vec2 pos = {0, 0};
		std::shared_ptr<const Window> window = shared_from_this();
		while (window){
			pos += window->pos;
			window = window->parent.lock();
		}
		return pos;
	}
	void Window::onLeftClick(int clicks){

	}
	void Window::onLeftRelease(){

	}
	void Window::onRightClick(int clicks){

	}
	void Window::onRightRelease(){

	}
	bool Window::leftMouseDown() const {
		if (inFocus()){
			return sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
		} else {
			return false;
		}
	}
	bool Window::rightMouseDown() const {
		if (inFocus()){
			return sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
		} else {
			return false;
		}
	}
	void Window::onScroll(float delta_x, float delta_y){

	}
	void Window::startDrag(){
		grabFocus();
		getContext().setDraggingWindow(weak_from_this(), (vec2)sf::Mouse::getPosition(getContext().getRenderWindow()) - pos);
	}
	void Window::onDrag(){
		
	}
	void Window::stopDrag(){
		if (dragging()){
			getContext().setDraggingWindow({});
		}
	}
	bool Window::dragging() const {
		return (getContext().getDraggingWindow().lock() == shared_from_this());
	}
	void Window::onHover(){

	}
	void Window::onHoverWithWindow(std::weak_ptr<Window> window){

	}
	void Window::drop(vec2 local_pos){
		vec2 pos = rootPos() + local_pos;
		if (auto window = root().findWindowAt(pos, weak_from_this()).lock()){
			auto self = weak_from_this();
			do {
				if (window->onDropWindow(self)){
					return;
				}
			} while (window = window->parent.lock());
		}
	}
	bool Window::onDropWindow(std::weak_ptr<Window> window){
		return false;
	}
	void Window::onFocus(){

	}
	bool Window::inFocus() const {
		return (getContext().getCurrentWindow().lock() == shared_from_this());
	}
	void Window::onLoseFocus(){

	}
	void Window::grabFocus(){
		getContext().focusTo(weak_from_this());
	}
	void Window::onKeyDown(Key key){

	}
	void Window::onKeyUp(Key key){

	}
	bool Window::keyDown(Key key) const {
		return inFocus() && sf::Keyboard::isKeyPressed(key);
	}
	void Window::remove(std::weak_ptr<Window> window){
		if (auto win = window.lock()){
			for (auto it = childwindows.begin(); it != childwindows.end(); ++it){
				if (*it == win){
					if ((*it)->inFocus()){
						grabFocus();
					}
					childwindows.erase(it);
					makeDirty();
					return;
				}
			}
		}
	}
	std::shared_ptr<Window> Window::release(std::weak_ptr<Window> window){
		if (auto win = window.lock()){
			for (auto it = childwindows.begin(); it != childwindows.end(); ++it){
				if (*it == win){
					std::shared_ptr<Window> child = *it;
					childwindows.erase(it);
					makeDirty();
					return child;
				}
			}
		}
		return nullptr;
	}
	void Window::bringToFront(){
		if (auto p = parent.lock()){
			auto self = shared_from_this();
			for (auto it = p->childwindows.begin(); it != p->childwindows.end(); ++it){
				if (*it == self){
					p->childwindows.erase(it);
					p->childwindows.insert(p->childwindows.begin(), self);
					return;
				}
			}
		}
	}
	void Window::clear(){
		childwindows.clear();
		makeDirty();
	}
	std::weak_ptr<Window> Window::findWindowAt(vec2 _pos, std::weak_ptr<Window> exclude){
		if (!visible || disabled){
			return {};
		}

		if (clipping && ((_pos.x < 0.0f) || (_pos.x > size.x) || (_pos.y < 0.0) || (_pos.y > size.y))){
			return {};
		}

		if (exclude.lock() == shared_from_this()){
			return {};
		}

		std::weak_ptr<Window> window;
		for (auto it = childwindows.begin(); it != childwindows.end(); ++it){
			window = (*it)->findWindowAt(_pos - (*it)->pos, exclude);
			if (auto win = window.lock()){
				return win;
			}
		}

		if (this->hit(_pos)){
			return weak_from_this();
		}

		return {};
	}
	void Window::render(sf::RenderWindow& renderwindow){
		sf::RectangleShape rectshape;
		rectshape.setSize(size);
		rectshape.setFillColor(sf::Color((((uint32_t)std::hash<Window*>{}(this)) & 0xFFFFFF00) | 0x80));
		rectshape.setOutlineColor(sf::Color(0xFF));
		rectshape.setOutlineThickness(1);
		renderwindow.draw(rectshape);
		renderChildWindows(renderwindow);
	}
	void Window::renderChildWindows(sf::RenderWindow& renderwindow){
		for (auto it = childwindows.begin(); it != childwindows.end(); ++it){
			const std::shared_ptr<Window>& child = *it;
			if (child->visible){
				if (child->clipping){
					getContext().translateView(child->pos);

					sf::FloatRect rect = getContext().getClipRect();
					vec2 pos = getContext().getViewOffset();
					getContext().intersectClipRect(sf::FloatRect(-pos, child->size));
					getContext().updateView();
					child->render(renderwindow);
					getContext().setClipRect(rect);
					getContext().translateView(-child->pos);
					getContext().updateView();
				} else {
					getContext().translateView(child->pos);
					getContext().updateView();
					child->render(renderwindow);
					getContext().translateView(-child->pos);
					getContext().updateView();
				}
			}
		}
	}
	std::vector<std::weak_ptr<Window>> Window::getChildWindows() const {
		std::vector<std::weak_ptr<Window>> ret;
		ret.reserve(childwindows.size());
		for (auto it = childwindows.begin(); it != childwindows.end(); ++it){
			ret.push_back(*it);
		}
		return ret;
	}
	std::weak_ptr<Window> Window::getParent() const {
		return parent;
	}

	void Window::makeDirty(){
		dirty = true;
	}

	bool Window::isDirty() const {
		return dirty;
	}

	bool Window::childrenDirty() const {
		for (const auto& child : childwindows){
			if (child->isDirty()){
				return true;
			}
		}
		return false;
	}

	void Window::makeClean(){
		dirty = false;
	}

	bool Window::update(float width_avail){
		// TODO: uncomment
		/*if (!isDirty()){
			for (auto child = childwindows.begin(); !isDirty() && child != childwindows.end(); ++child){
				if ((*child)->update((*child)->size.x)){
					makeDirty();
				}
			}
			if (!isDirty()){
				return false;
			}
		}*/

		makeClean();
		// calculate own width and arrange children
		if (display_style == DisplayStyle::Free){
			arrangeChildren(size.x);
			return false;
		} else {
			vec2 newsize = arrangeChildren(width_avail);
			if (display_style == DisplayStyle::Block){
				newsize.x = width_avail;
			}
			size = vec2(
				std::max(newsize.x, min_size.x),
				std::max(newsize.y, min_size.y)
			);
			float diff = abs(newsize.x - size.x) + abs(newsize.y - size.y);
			return diff > epsilon;
		}
	}

	vec2 Window::arrangeChildren(float width_avail){
		// TODO: make member
		const float padding = 5.0f;

		vec2 contentsize = {0, 0};
		float ypos = padding;
		float next_ypos = ypos;
		float xpos = padding;

		for (const auto& window : childwindows){
			switch (window->display_style){
				case DisplayStyle::Block:
					// block elements appear on a new line and may take up the full
					// width available and as much height as needed
					{
						xpos = padding;
						window->setPos({xpos, next_ypos});
						float avail = width_avail - 2.0f * padding;
						window->update(avail);
						ypos = next_ypos + window->getSize().y + padding;
						next_ypos = ypos;
					}
					break;
				case DisplayStyle::Inline:
				case DisplayStyle::InlineBlock:
					// inline elements appear inline and will take up only as much space as needed
					// inline-block elements appear inline but may take up any desired amount of space
					{
						window->setPos({xpos, ypos});
						float avail = width_avail - padding - xpos;
						window->update(avail);
						// if the window exceeds the end of the line, put it on a new line and rearrange
						if (xpos + window->getSize().x + padding > width_avail){
							xpos = padding;
							ypos = next_ypos;
							avail = width_avail - 2.0f * padding;
							window->setPos({xpos, ypos});
							window->update(avail);
						}
						xpos += window->size.x + padding;
						next_ypos = std::max(next_ypos, ypos + window->getSize().y + padding);
					}
					break;
				case DisplayStyle::Free:
					// free elements do not appear as flow elements but are positioned
					// relative to their parent at their x/y position (like the classic ui)
					
					// TODO:
					break;
			}
			contentsize = vec2(
				std::max(contentsize.x, window->pos.x + window->size.x + padding),
				std::max(contentsize.y, window->pos.y + window->size.y + padding)
			);
		}

		return contentsize;
	}

	FreeElement::FreeElement() : Window(DisplayStyle::Free) {

	}

	InlineElement::InlineElement() : Window(DisplayStyle::Inline) {

	}

	BlockElement::BlockElement() : Window(DisplayStyle::Block) {

	}

	InlineBlockElement::InlineBlockElement() : Window(DisplayStyle::InlineBlock) {

	}

}