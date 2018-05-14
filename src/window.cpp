#include "gui/window.h"
#include "gui/gui.h"
#include <set>

namespace ui {

	Window::~Window(){
		childwindows.clear();
		if (inFocus()){
			getContext().focusTo(parent);
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
	void Window::onScroll(double delta_x, double delta_y){

	}
	void Window::startDrag(){
		grabFocus();
		getContext().setDraggingWindow(weak_from_this(), (vec2)sf::Mouse::getPosition(getContext().getRenderWindow()) - pos);
	}
	void Window::onDrag(){
		
	}
	void Window::stopDrag() const {
		if (dragging()){
			getContext().setDraggingWindow({});
		}
	}
	bool Window::dragging() const {
		return (getContext().getDraggingWindow().lock() == shared_from_this());
	}
	void Window::onHover(){

	}
	void Window::onHoverWithWindow(std::weak_ptr<Window> drag_window){

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
	void Window::onKeyDown(sf::Keyboard::Key key){

	}
	void Window::onKeyUp(sf::Keyboard::Key key){

	}
	bool Window::keyDown(sf::Keyboard::Key key) const {
		return inFocus() && sf::Keyboard::isKeyPressed(key);
	}
	void Window::remove(std::weak_ptr<Window> window){
		if (auto win = window.lock()){
			for (auto it = childwindows.begin(); it != childwindows.end(); ++it){
				if (*it == win){
					childwindows.erase(it);
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
	}
	std::weak_ptr<Window> Window::findWindowAt(vec2 _pos){
		if (!visible || disabled){
			return {};
		}

		if (clipping && ((_pos.x < 0.0f) || (_pos.x > size.x) || (_pos.y < 0.0) || (_pos.y > size.y))){
			return {};
		}

		std::weak_ptr<Window> window;
		for (auto it = childwindows.begin(); it != childwindows.end(); ++it){
			window = (*it)->findWindowAt(_pos - (*it)->pos);
			if (auto win = window.lock()){
				if (!win->dragging()){
					return win;
				}
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
		for (auto it = childwindows.rbegin(); it != childwindows.rend(); ++it){
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

}