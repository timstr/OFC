#include "gui/element.h"
#include "gui/gui.h"
#include <algorithm>
#include <set>

namespace ui {
	namespace {
		const float epsilon = 0.0001f;
		const float far_away = 1000000.0f;
	}

	Element::Element(DisplayStyle _display_style) :
		shared_this(this),
		display_style(_display_style),
		pos({0.0f, 0.0f}),
		size({100.0f, 100.0f}),
		min_size({0.0f, 0.0f}),
		max_size({far_away, far_away}),
		old_total_size({0.0f, 0.0f}),
		disabled(false),
		visible(true),
		clipping(false),
		dirty(true),
		layout_index(0.0f),
		padding(5.0f),
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
			if (min_size.x > size.x || min_size.y > size.y || max_size.x < size.x || max_size.y < size.y){
				makeDirty();
			}
			min_size = _size;
			max_size = _size;
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

	void Element::setDisplayStyle(DisplayStyle style){
		if (display_style != style){
			display_style = style;
			if (auto par = parent.lock()){
				par->makeDirty();
			}
		}
	}

	DisplayStyle Element::getDisplayStyle() const {
		return display_style;
	}
	
	Element::~Element(){
		
	}
	
	void Element::close(){
		onClose();
		while (!children.empty()){
			if (children.back()->inFocus()){
				grabFocus();
			}
			children.back()->close();
		}
		if (auto p = parent.lock()){
			p->remove(shared_this);
		}
		shared_this = nullptr;
	}

	bool Element::isClosed() const {
		return !static_cast<bool>(shared_this);
	}

	void Element::onClose(){

	}
	
	bool Element::hit(vec2 testpos) const {
		return ((testpos.x >= 0.0f) && (testpos.x < size.x) && (testpos.y >= 0.0f) && (testpos.y < size.y));
	}
	
	vec2 Element::localMousePos() const {
		vec2 pos = (vec2)sf::Mouse::getPosition(getContext().getRenderWindow());
		std::shared_ptr<const Element> element = shared_this;
		while (element){
			pos -= element->pos;
			element = element->parent.lock();
		}
		return pos;
	}
	
	vec2 Element::rootPos() const {
		vec2 pos = {0, 0};
		std::shared_ptr<const Element> element = shared_this;
		while (element){
			pos += element->pos;
			element = element->parent.lock();
		}
		return pos;
	}
	
	bool Element::onLeftClick(int clicks){
		return false;
	}
	
	void Element::onLeftRelease(){
		
	}
	
	bool Element::onRightClick(int clicks){
		return false;
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
	
	bool Element::onScroll(float delta_x, float delta_y){
		return false;
	}
	
	void Element::startDrag(){
		if (display_style == DisplayStyle::Free){
			grabFocus();
			getContext().setDraggingElement(shared_this, (vec2)sf::Mouse::getPosition(getContext().getRenderWindow()) - pos);
		}
	}
	
	void Element::onDrag(){
		
	}
	
	void Element::stopDrag(){
		if (dragging()){
			getContext().setDraggingElement(nullptr);
		}
	}
	
	bool Element::dragging() const {
		return (getContext().getDraggingElement().get() == this);
	}

	void Element::onMouseOver(){

	}

	void Element::onMouseOut(){

	}

	bool Element::hovering() const {
		return getContext().getHoverElement().get() == this;
	}
	
	bool Element::onHover(){
		return false;
	}
	
	bool Element::onHoverWith(std::shared_ptr<Element> element){
		return false;
	}
	
	void Element::drop(vec2 local_pos){
		vec2 pos = rootPos() + local_pos;
		if (auto element = root().findElementAt(pos, shared_this)){
			do {
				if (element->onDrop(shared_this)){
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
		return (getContext().getCurrentElement().get() == this);
	}
	
	void Element::onLoseFocus(){

	}
	
	void Element::grabFocus(){
		getContext().focusTo(shared_this);
	}
	
	bool Element::onKeyDown(Key key){
		return false;
	}
	
	void Element::onKeyUp(Key key){
		
	}
	
	bool Element::keyDown(Key key) const {
		return inFocus() && sf::Keyboard::isKeyPressed(key);
	}
	
	void Element::adopt(std::shared_ptr<Element> child){
		if (auto p = child->parent.lock()){
			if (p.get() == this){
				return;
			}
			p->release(child);
		}
		children.push_back(child);
		child->parent = shared_this;
		child->layout_index = getNextLayoutIndex();
		organizeLayoutIndices();
		makeDirty();
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
			for (auto it = p->children.begin(); it != p->children.end(); ++it){
				if ((*it).get() == this){
					p->children.erase(it);
					p->children.push_back(shared_this);
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

		if (exclude.get() == this){
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
			return shared_this;
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
	
	float Element::getNextLayoutIndex() const {
		float max = 0.0f;
		for (const auto& child : children){
			max = std::max(child->layout_index, max);
		}
		return max + 1.0f;
	}
		
	void Element::organizeLayoutIndices(){
		std::set<std::pair<float, std::shared_ptr<Element>>> index_set;
		for (const auto& child : children){
			index_set.insert({child->layout_index, child});
		}
		float i = 0.0f;
		for (const auto& mapping : index_set){
			mapping.second->layout_index = i;
			i += 1.0f;
		}
	}
	
	const std::vector<std::shared_ptr<Element>>& Element::getChildren() const {
		return children;
	}
	
	std::weak_ptr<Element> Element::getParent() const {
		return parent;
	}

	void Element::layoutBefore(const std::shared_ptr<Element>& sibling){
		if (!sibling || isClosed()){
			return;
		}
		if (auto mypar = parent.lock()){
			if (auto otherpar = sibling->parent.lock()){
				if (mypar != otherpar){
					return;
				}
				layout_index = sibling->layout_index - 0.5f;
				organizeLayoutIndices();
			}
		}
	}

	void Element::layoutAfter(const std::shared_ptr<Element>& sibling){
		if (!sibling || isClosed()){
			return;
		}
		if (auto mypar = parent.lock()){
			if (auto otherpar = sibling->parent.lock()){
				if (mypar != otherpar){
					return;
				}
				layout_index = sibling->layout_index + 0.5f;
				organizeLayoutIndices();
			}
		}
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
				newsize = arrangeChildren(size.x);
				size = vec2(
					std::min(std::max(newsize.x, min_size.x), max_size.x),
					std::min(std::max(newsize.y, min_size.y), max_size.y)
				);
			}
			
			vec2 new_total_size = size + vec2(2.0f * margin, 2.0f * margin);
			float diff = abs(old_total_size.x - new_total_size.x) + abs(old_total_size.y - new_total_size.y);
			old_total_size = new_total_size;
			return diff > epsilon;
		}
	}

	struct LayoutData {
		LayoutData(Element& _self, float _width_avail)
			: self(_self),
			width_avail(_width_avail),
			sorted_elements(_self.getChildren()) {

			reset();
			auto comp = [](const std::shared_ptr<Element>& l, const std::shared_ptr<Element>& r){
				return l->layout_index < r->layout_index;
			};
			std::sort(sorted_elements.begin(), sorted_elements.end(), comp);
		}

		const Element& self;
		float width_avail;
		vec2 contentsize;
		float xpos;
		float ypos;
		float next_ypos;
		float left_edge, right_edge;
		bool emptyline;
		std::vector<std::shared_ptr<Element>> sorted_elements, floatingleft, floatingright;
		
		void reset(){
			xpos = self.padding;
			ypos = self.padding;
			next_ypos = self.padding;
			contentsize = {2.0f * self.padding, 2.0f * self.padding};
			left_edge = self.padding;
			right_edge = width_avail - self.padding;
			emptyline = true;
		}

		void layoutElements(){
			std::vector<std::shared_ptr<Element>> left_elems, right_elems, inline_elems;

			// arranges and empties the current left- and right- floating elements
			// and inline elements, returning the maximum width needed or the current
			// available width if it would be exceeded without breaking onto a new line
			auto layoutBatch = [&,this]() -> float {
				float lwidth = 0.0f;
				float rwidth = 0.0f;
				float iwidth = 0.0f;
				bool broke_line = false;
				for (const auto& elem : left_elems){
					if (arrangeFloatingLeft(elem)){
						broke_line = true;
					} else {
						lwidth += elem->size.x + 2.0f * elem->margin;
					}
				}
				for (const auto& elem : right_elems){
					if (arrangeFloatingRight(elem)){
						broke_line = true;
					} else {
						lwidth += elem->size.x + 2.0f * elem->margin;
					}
				}
				for (const auto& elem : inline_elems){
					if (arrangeInline(elem)){
						broke_line = true;
					} else {
						iwidth += elem->size.x + 2.0f * elem->margin;
					}
				}
				left_elems.clear();
				right_elems.clear();
				inline_elems.clear();
				if (broke_line){
					return this->width_avail;
				} else {
					return lwidth + rwidth + iwidth + 2.0f * self.padding;
				}
			};

			// arrange all elements in batches to fit within the available width,
			// returning the maximum width needed without breaking lines
			// or the available width if it would be exceeded
			auto layoutEverything = [&,this]() -> float {
				float max_width = 0.0f;
				auto it = sorted_elements.cbegin();
				auto end = sorted_elements.cend();
				while (true){
					if (it == end){
						max_width = std::max(max_width, layoutBatch());
						break;
					}

					std::shared_ptr<Element> elem = *it;

					if (elem->visible){
						switch (elem->display_style){
							case DisplayStyle::Block:
								max_width = std::max(max_width, layoutBatch());
								arrangeBlock(elem);
								break;
							case DisplayStyle::Inline:
								inline_elems.push_back(elem);
								break;
							case DisplayStyle::FloatLeft:
								left_elems.push_back(elem);
								break;
							case DisplayStyle::FloatRight:
								right_elems.push_back(elem);
								break;
						}
					}

					++it;
				}

				return max_width;
			};

			float max_width = layoutEverything();

			if (max_width < width_avail && self.display_style != DisplayStyle::Free && self.display_style != DisplayStyle::Block){
				width_avail = max_width;
				reset();
				layoutEverything();
			}
		}
		
		void arrangeBlock(const std::shared_ptr<Element>& element){
			Element& elem = *element;

			while (nextWiderLine()){

			}

			elem.setPos({self.padding + elem.margin, next_ypos + elem.margin});
			elem.update(right_edge - left_edge - 2.0f * elem.margin);
			next_ypos = elem.pos.y + elem.size.y + elem.margin;
			fitContents(elem);
			newLine();
		}

		// arranges an inline element adjacent to previous inline elements,
		// flowing around floating elements.
		// returns true if the available width was exceeded and the element
		// broke onto a new line
		bool arrangeInline(const std::shared_ptr<Element>& element){
			Element& elem = *element;
			bool broke_line = false;
			do {
				// position the element
				elem.setPos({xpos + elem.margin, ypos + elem.margin});
				elem.update(right_edge - xpos - 2.0f * elem.margin - self.padding);

				if (elem.pos.x + elem.size.x + elem.margin > right_edge){
					broke_line = true;
					// if it goes past the edge
					if (emptyline){
						// if it's the only inline element on the line, find the next wider line
						if (!nextWiderLine()){
							// if one doesn't exist
							break;
						}
					} else {
						// if other inline elements are on the line
						newLine();
					}
				} else {
					// if the element fits
					break;
				}

			} while (true);

			xpos = elem.pos.x + elem.size.x + elem.margin;
			next_ypos = std::max(next_ypos, elem.pos.y + elem.size.y + elem.margin);
			fitContents(elem);
			emptyline = false;
			return broke_line;
		}

		// arranges a left-floating element to the right of the left edge
		// and any current left-floating elements.
		// returns true if the available width was exceeded and the element
		// broke onto a new line
		bool arrangeFloatingLeft(const std::shared_ptr<Element>& element){
			if (!emptyline){
				newLine();
			}
			Element& elem = *element;
			bool broke_line = false;
			do {
				elem.setPos({left_edge + elem.margin, ypos + elem.margin});
				elem.update(right_edge - left_edge - 2.0f * elem.margin);
				if (elem.pos.x + elem.size.x + elem.padding > right_edge){
					broke_line = true;
					// if the element doesn't fit
					if (!nextWiderLine()){
						// if a wider line can't be found
						break;
					}
				} else {
					// if the element fits
					break;
				}
			} while(true);
			floatingleft.push_back(element);
			left_edge = getLeftEdge();
			xpos = left_edge;
			fitContents(elem);
			return broke_line;
		}

		// arranges a right-floating element left of the right edge and any
		// other current right-floating elements.
		// return true if the available width was exceeded and the element
		// broke onto a new line
		bool arrangeFloatingRight(const std::shared_ptr<Element>& element){
			Element& elem = *element;
			bool broke_line = false;
			do {
				float avail = right_edge - left_edge - elem.margin * 2.0f;
				elem.update(avail);
				if (elem.getSize().x > avail){
					broke_line = true;
					if (!nextWiderLine()){
						// if a wider line can't be found
						break;
					}
				} else {
					break;
				}
			} while(true);

			elem.setPos({right_edge - elem.size.x - elem.margin, ypos + elem.margin});
			floatingright.push_back(element);
			right_edge = getRightEdge();
			fitContents(elem);
			return broke_line;
		}

		// adjusts `contentsize` to include the element
		void fitContents(const Element& elem){
			contentsize = vec2(
				std::max(contentsize.x, elem.pos.x + elem.size.x + elem.margin + self.padding),
				std::max(contentsize.y, elem.pos.y + elem.size.y + elem.margin + self.padding)
			);
		}

		bool nextWiderLine(){
			// if floating left elements, find next bottom edge of floating left and make newline there, return true
			// else return false

			if (floatingleft.size() > 0 || floatingright.size() > 0){
				float next_y = far_away;
				for (const auto& elem : floatingleft){
					next_y = std::min(next_y, elem->pos.y + elem->size.y + elem->margin);
				}

				for (const auto& elem : floatingright){
					next_y = std::min(next_y, elem->pos.y + elem->size.y + elem->margin);
				}

				next_ypos = std::max(next_y, next_ypos);
				newLine();
				return true;
			}

			return false;
		}

		void newLine(){
			ypos = next_ypos;

			auto aboveNewLine = [=](const std::shared_ptr<Element>& elem){
				return ypos >= elem->getPos().y + elem->getSize().y + elem->getMargin();
			};

			floatingleft.erase(
				std::remove_if(floatingleft.begin(), floatingleft.end(), aboveNewLine),
				floatingleft.end()
			);

			floatingright.erase(
				std::remove_if(floatingright.begin(), floatingright.end(), aboveNewLine),
				floatingright.end()
			);

			left_edge = getLeftEdge();
			right_edge = getRightEdge();
			xpos = left_edge;
			emptyline = true;
		}

		float getLeftEdge() const {
			// Assumption: all elements in `floatingleft` pass through ypos
			float extent = self.padding;
			for (const auto& elem : floatingleft){
				extent = std::max(extent, elem->getPos().x + elem->getSize().x + elem->getMargin());
			}
			return extent;
		}

		float getRightEdge() const {
			// Assumption: all elements in `floatingright` pass through ypos
			float extent = width_avail - self.padding;
			for (const auto& elem : floatingright){
				extent = std::min(extent, elem->getPos().x - elem->getMargin());
			}
			return extent;
		}
	};

	vec2 Element::arrangeChildren(float width_avail){
		
		LayoutData layout(*this, width_avail);

		layout.layoutElements();

		return layout.contentsize;
	}

	FreeElement::FreeElement() : Element(DisplayStyle::Free) {

	}

	InlineElement::InlineElement() : Element(DisplayStyle::Inline) {

	}

	BlockElement::BlockElement() : Element(DisplayStyle::Block) {

	}

	LeftFloatingElement::LeftFloatingElement() : Element(DisplayStyle::FloatLeft) {

	}

	RightFloatingElement::RightFloatingElement() : Element(DisplayStyle::FloatRight) {

	}

}