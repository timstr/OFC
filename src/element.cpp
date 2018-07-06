#include "gui/element.h"
#include "gui/gui.h"
#include "gui/text.h"
#include "gui/roundrectangle.h"

#include <algorithm>
#include <set>
#include <sstream>

namespace ui {
	namespace {
		const float epsilon = 0.0001f;
		const float far_away = 1000000.0f;
	}

	Element::Element(LayoutStyle _display_style) :
		shared_this(this),
		layout_style(_display_style),
		content_align(ContentAlign::Left),
		pos({ 0.0f, 0.0f }),
		size({ 100.0f, 100.0f }),
		min_size({ 0.0f, 0.0f }),
		max_size({ far_away, far_away }),
		old_total_size({ 0.0f, 0.0f }),
		disabled(false),
		visible(true),
		clipping(false),
		dirty(true),
		layout_index(0.0f),
		padding(0.0f),
		margin(0.0f) {

		display_rect.setSize(size);
		display_rect.setFillColor(sf::Color(0));
		display_rect.setOutlineThickness(0.0f);
		display_rect.setOutlineColor(sf::Color(0xFF));
		display_rect.setPosition({ 1.0f, 1.0f });
	}

	void Element::disable() {
		disabled = true;
	}

	void Element::enable() {
		disabled = false;
	}

	bool Element::isEnabled() const {
		return !disabled;
	}

	void Element::setVisible(bool is_visible) {
		if (visible != is_visible && layout_style != LayoutStyle::Free) {
			makeDirty();
		}
		visible = is_visible;
	}

	bool Element::isVisible() const {
		return visible;
	}

	void Element::setClipping(bool _clipping) {
		clipping = _clipping;
	}

	vec2 Element::getPos() const {
		return pos;
	}

	void Element::setPos(vec2 _pos) {
		if (abs(pos.x - _pos.x) + abs(pos.y - _pos.y) > epsilon) {
			pos = _pos;
		}
	}

	vec2 Element::getSize() const {
		return size;
	}

	void Element::setSize(vec2 _size, bool force) {
		_size = vec2(std::max(_size.x, 0.0f), std::max(_size.y, 0.0f));
		if (abs(size.x - _size.x) + abs(size.y - _size.y) > epsilon) {
			size = _size;
			makeDirty();
		}
		if (force) {
			if (min_size.x > size.x || min_size.y > size.y || max_size.x < size.x || max_size.y < size.y) {
				makeDirty();
			}
			min_size = _size;
			max_size = _size;
		}
	}

	void Element::setMinSize(vec2 _min_size) {
		_min_size = vec2(std::max(_min_size.x, 0.0f), std::max(_min_size.y, 0.0f));
		min_size = _min_size;
		if (min_size.x > size.x || min_size.y > size.y) {
			makeDirty();
		}
	}

	vec2 Element::getMaxSize() const {
		return max_size;
	}

	vec2 Element::getMinSize() const {
		return min_size;
	}

	void Element::setMinWidth(float width) {
		setMinSize({ width, min_size.y });
	}
	void Element::setMinHeight(float height) {
		setMinSize({ min_size.x, height });
	}

	void Element::setMaxSize(vec2 _max_size) {
		_max_size = vec2(std::max(_max_size.x, 0.0f), std::max(_max_size.y, 0.0f));
		max_size = _max_size;
		if (max_size.x < size.x || max_size.y < size.y) {
			makeDirty();
		}
	}

	void Element::setMaxWidth(float width) {
		setMaxSize({ width, max_size.y });
	}
	void Element::setWidth(float width, bool force) {
		width = std::max(0.0f, width);
		if (abs(size.x - width) > epsilon) {
			size.x = width;
			makeDirty();
		}
		if (force) {
			if (size.x < min_size.x || size.x > max_size.x) {
				makeDirty();
			}
			min_size.x = width;
			max_size.x = width;
		}
	}
	void Element::setMaxHeight(float height) {
		setMaxSize({ max_size.x, height });
	}

	void Element::setHeight(float height, bool force) {
		height = std::max(0.0f, height);
		if (abs(size.y - height) > epsilon) {
			size.y = height;
			makeDirty();
		}
		if (force) {
			if (size.y < min_size.y || size.y > max_size.y) {
				makeDirty();
			}
			min_size.y = height;
			max_size.y = height;
		}
	}

	void Element::onResize() {

	}

	void Element::setLayoutStyle(LayoutStyle style) {
		if (layout_style != style) {
			layout_style = style;
			if (auto par = parent.lock()) {
				par->makeDirty();
			}
		}
	}

	LayoutStyle Element::getLayoutStyle() const {
		return layout_style;
	}

	void Element::setContentAlign(ContentAlign style) {
		if (content_align != style) {
			content_align = style;
			makeDirty();
		}
	}

	ContentAlign Element::getContentAlign() const {
		return content_align;
	}

	void Element::setXPositionStyle(PositionStyle style, float spacing) {
		x_position_style = style;
		x_spacing = spacing;
	}

	PositionStyle Element::getXPositionStyle() const {
		return x_position_style;
	}

	float Element::getXPositionSpacing() const {
		return x_spacing;
	}

	void Element::setYPositionStyle(PositionStyle style, float spacing) {
		y_position_style = style;
		y_spacing = spacing;
	}

	PositionStyle Element::getYPositionStyle() const {
		return y_position_style;
	}

	float Element::getYPositionSpacing() const {
		return y_spacing;
	}

	Element::~Element() {

	}

	void Element::close() {
		onClose();
		while (!children.empty()) {
			if (children.back()->inFocus()) {
				grabFocus();
			}
			children.back()->close();
		}
		if (auto p = parent.lock()) {
			p->remove(shared_this);
		}
		shared_this = nullptr;
	}

	bool Element::isClosed() const {
		return !static_cast<bool>(shared_this);
	}

	void Element::onClose() {

	}

	bool Element::hit(vec2 testpos) const {
		return ((testpos.x >= 0.0f) && (testpos.x < size.x) && (testpos.y >= 0.0f) && (testpos.y < size.y));
	}

	vec2 Element::localMousePos() const {
		vec2 pos = (vec2)sf::Mouse::getPosition(getContext().getRenderWindow());
		std::shared_ptr<const Element> element = shared_this;
		while (element) {
			pos -= element->pos;
			element = element->parent.lock();
		}
		return pos;
	}

	vec2 Element::rootPos() const {
		vec2 pos = { 0, 0 };
		std::shared_ptr<const Element> element = shared_this;
		while (element) {
			pos += element->pos;
			element = element->parent.lock();
		}
		return pos;
	}

	bool Element::onLeftClick(int clicks) {
		return false;
	}

	void Element::onLeftRelease() {

	}

	bool Element::onRightClick(int clicks) {
		return false;
	}

	void Element::onRightRelease() {

	}

	bool Element::leftMouseDown() const {
		if (inFocus()) {
			return sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
		} else {
			return false;
		}
	}

	bool Element::rightMouseDown() const {
		if (inFocus()) {
			return sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
		} else {
			return false;
		}
	}

	bool Element::onScroll(float delta_x, float delta_y) {
		return false;
	}

	void Element::startDrag() {
		if (layout_style == LayoutStyle::Free) {
			grabFocus();
			getContext().setDraggingElement(shared_this, (vec2)sf::Mouse::getPosition(getContext().getRenderWindow()) - pos);
		}
	}

	void Element::onDrag() {

	}

	void Element::stopDrag() {
		if (dragging()) {
			getContext().setDraggingElement(nullptr);
		}
	}

	bool Element::dragging() const {
		return (getContext().getDraggingElement().get() == this);
	}

	void Element::onMouseOver() {

	}

	void Element::onMouseOut() {

	}

	bool Element::hovering() const {
		return getContext().getHoverElement().get() == this;
	}

	bool Element::onHover() {
		return false;
	}

	bool Element::onHoverWith(std::shared_ptr<Element> element) {
		return false;
	}

	void Element::drop(vec2 local_pos) {
		vec2 pos = rootPos() + local_pos;
		if (auto element = root().findElementAt(pos, shared_this)) {
			do {
				if (element->onDrop(shared_this)) {
					return;
				}
			} while (element = element->parent.lock());
		}
	}

	bool Element::onDrop(std::shared_ptr<Element> element) {
		return false;
	}

	void Element::onFocus() {

	}

	bool Element::inFocus() const {
		return (getContext().getCurrentElement().get() == this);
	}

	void Element::onLoseFocus() {

	}

	void Element::grabFocus() {
		getContext().focusTo(shared_this);
	}

	bool Element::onKeyDown(Key key) {
		return false;
	}

	void Element::onKeyUp(Key key) {

	}

	bool Element::keyDown(Key key) const {
		return inFocus() && sf::Keyboard::isKeyPressed(key);
	}

	void Element::write(const std::string& text, sf::Font& font, sf::Color color, unsigned charsize, TextStyle style) {
		write(std::wstring { text.begin(), text.end() }, font, color, charsize, style);
	}

	void Element::write(const std::wstring& text, sf::Font& font, sf::Color color, unsigned charsize, TextStyle style) {
		std::wstring word;

		auto writeWord = [&, this]() {
			if (word.size() > 0) {
				this->add<ui::Text>(word, font, color, charsize, style);
				word.clear();
			}
		};

		for (const wchar_t& ch : text) {
			if (ch == L'\n') {
				writeWord();
				writeLineBreak(charsize);
			} else if (ch == L'\t') {
				writeWord();
				writeTab();
			} else if (ch == L' ') {
				writeWord();
			} else {
				word += ch;
			}
		}
		writeWord();
	}

	void Element::writeLineBreak(unsigned charsize) {
		white_spaces.push_back(WhiteSpace(WhiteSpace::LineBreak, getNextLayoutIndex(), charsize));
		makeDirty();
	}

	void Element::writePageBreak(float height) {
		auto br = add<BlockElement>();
		br->setPadding(0.0f);
		br->setMargin(height * 0.5f);
		br->setBorderColor(sf::Color(0));
		br->setBackgroundColor(sf::Color(0));
		br->disable();
		br->setSize({ 0.0f, 0.0f });
	}

	void Element::writeTab(float width) {
		unsigned charsize = static_cast<unsigned>(floor(width / 50.0f * 15.0f));
		white_spaces.push_back(WhiteSpace(WhiteSpace::Tab, getNextLayoutIndex(), charsize));
		makeDirty();
	}

	void Element::adopt(std::shared_ptr<Element> child) {
		if (auto p = child->parent.lock()) {
			if (p.get() == this) {
				return;
			}
			p->release(child);
		}
		children.push_back(child);
		child->parent = shared_this;
		child->layout_index = getNextLayoutIndex();
		makeDirty();
		if (layout_style != LayoutStyle::Free)
			if (auto p = parent.lock()) {
				p->makeDirty();
			}
	}

	void Element::remove(std::shared_ptr<Element> element) {
		if (element) {
			for (auto it = children.begin(); it != children.end(); ++it) {
				if (*it == element) {
					if (element->inFocus()) {
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

	std::shared_ptr<Element> Element::release(std::shared_ptr<Element> element) {
		if (element) {
			for (auto it = children.begin(); it != children.end(); ++it) {
				if (*it == element) {
					children.erase(it);
					element->parent = {};
					makeDirty();
					return element;
				}
			}
		}
		return nullptr;
	}

	void Element::bringToFront() {
		if (auto p = parent.lock()) {
			for (auto it = p->children.begin(); it != p->children.end(); ++it) {
				if ((*it).get() == this) {
					p->children.erase(it);
					p->children.push_back(shared_this);
					return;
				}
			}
		}
	}

	void Element::clear() {
		children.clear();
		makeDirty();
	}

	std::shared_ptr<Element> Element::findElementAt(vec2 _pos, std::shared_ptr<Element> exclude) {
		if (!visible || disabled) {
			return nullptr;
		}

		if (clipping && ((_pos.x < 0.0f) || (_pos.x > size.x) || (_pos.y < 0.0) || (_pos.y > size.y))) {
			return nullptr;
		}

		if (exclude.get() == this) {
			return nullptr;
		}

		std::shared_ptr<Element> element;
		for (auto it = children.rbegin(); it != children.rend(); ++it) {
			element = (*it)->findElementAt(_pos - (*it)->pos, exclude);
			if (element) {
				return element;
			}
		}

		if (this->hit(_pos)) {
			return shared_this;
		}

		return nullptr;
	}

	void Element::render(sf::RenderWindow& rw) const {
		rw.draw(display_rect);
	}

	void Element::renderChildren(sf::RenderWindow& renderwindow) {
		for (auto it = children.begin(); it != children.end(); ++it) {
			const std::shared_ptr<Element>& child = *it;
			if (child->visible) {
				if (child->clipping) {
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

	Element::LayoutIndex Element::getNextLayoutIndex() const {
		LayoutIndex max = 0.0f;
		for (const auto& child : children) {
			max = std::max(child->layout_index, max);
		}
		for (const auto& space : white_spaces) {
			max = std::max(space.layout_index, max);
		}
		return max + 1.0f;
	}

	void Element::organizeLayoutIndices() {
		using ElementOrSpace = std::pair<std::shared_ptr<Element>, WhiteSpace*>;

		std::set<std::pair<LayoutIndex, ElementOrSpace>> index_set;
		for (const auto& child : children) {
			index_set.insert({ child->layout_index, {child, nullptr} });
		}
		for (auto& space : white_spaces) {
			index_set.insert({ space.layout_index, {nullptr, &space} });
		}

		LayoutIndex i = 0.0f;
		for (const auto& mapping : index_set) {
			if (mapping.second.first) {
				mapping.second.first->layout_index = i;
			} else if (mapping.second.second) {
				mapping.second.second->layout_index = i;
			}
			i += 1.0f;
		}
	}

	const std::vector<std::shared_ptr<Element>>& Element::getChildren() const {
		return children;
	}

	std::weak_ptr<Element> Element::getParent() const {
		return parent;
	}

	void Element::layoutBefore(const std::shared_ptr<Element>& sibling) {
		if (!sibling || isClosed()) {
			return;
		}
		if (auto mypar = parent.lock()) {
			if (auto otherpar = sibling->parent.lock()) {
				if (mypar != otherpar) {
					return;
				}
				layout_index = sibling->layout_index - 0.5f;
				organizeLayoutIndices();
			}
		}
	}

	void Element::layoutAfter(const std::shared_ptr<Element>& sibling) {
		if (!sibling || isClosed()) {
			return;
		}
		if (auto mypar = parent.lock()) {
			if (auto otherpar = sibling->parent.lock()) {
				if (mypar != otherpar) {
					return;
				}
				layout_index = sibling->layout_index + 0.5f;
				organizeLayoutIndices();
			}
		}
	}

	void Element::setPadding(float _padding) {
		if (abs(padding - _padding) > epsilon) {
			padding = std::max(_padding, 0.0f);
			makeDirty();
		}
	}

	float Element::getPadding() const {
		return padding;
	}

	void Element::setMargin(float _margin) {
		_margin = std::max(_margin, 0.0f);
		if (abs(margin - _margin) > epsilon) {
			margin = _margin;
			makeDirty();
		}
	}

	float Element::getMargin() const {
		return margin;
	}

	sf::Color Element::getBackgroundColor() const {
		return display_rect.getFillColor();
	}

	void Element::setBackgroundColor(sf::Color color) {
		display_rect.setFillColor(color);
	}

	sf::Color Element::getBorderColor() const {
		return display_rect.getOutlineColor();
	}

	void Element::setBorderColor(sf::Color color) {
		display_rect.setOutlineColor(color);
	}

	float Element::getBorderRadius() const {
		return display_rect.getRadius();
	}

	void Element::setBorderRadius(float radius) {
		display_rect.setRadius(radius);
	}

	float Element::getBorderThickness() const {
		return display_rect.getOutlineThickness();
	}

	void Element::setBorderThickness(float thickness) {
		display_rect.setOutlineThickness(std::max(0.0f, thickness));
	}

	void Element::updatePosition() {
		if (layout_style != LayoutStyle::Free) {
			return;
		}
		auto par = parent.lock();
		if (!par) {
			return;
		}
		switch (x_position_style) {
			case PositionStyle::OutsideBegin:
				pos.x = -x_spacing - size.x;
				break;
			case PositionStyle::InsideBegin:
				pos.x = x_spacing;
				break;
			case PositionStyle::Center:
				pos.x = (par->size.x - size.x) * 0.5f;
				break;
			case PositionStyle::InsideEnd:
				pos.x = par->size.x - size.x - x_spacing;
				break;
			case PositionStyle::OutsideEnd:
				pos.x = par->size.x + x_spacing;
				break;
		}
		switch (y_position_style) {
			case PositionStyle::OutsideBegin:
				pos.y = -y_spacing - size.y;
				break;
			case PositionStyle::InsideBegin:
				pos.y = y_spacing;
				break;
			case PositionStyle::Center:
				pos.y = (par->size.y - size.y) * 0.5f;
				break;
			case PositionStyle::InsideEnd:
				pos.y = par->size.y - size.y - y_spacing;
				break;
			case PositionStyle::OutsideEnd:
				pos.y = par->size.y + y_spacing;
				break;
		}
	}

	void Element::updateChildPositions() {
		for (const auto& child : children) {
			child->updatePosition();
		}
	}

	void Element::makeDirty() {
		dirty = true;
	}

	bool Element::isDirty() const {
		return dirty;
	}

	void Element::makeClean() {
		dirty = false;
	}

	bool Element::update(float width_avail) {
		width_avail = std::min(std::max(width_avail, min_size.x), max_size.x);

		if (layout_style == LayoutStyle::Block) {
			setSize({
				width_avail,
				size.y
			});
		}

		if (!isDirty()) {
			for (auto child = children.begin(); !isDirty() && child != children.end(); ++child) {
				if ((*child)->update((*child)->size.x)) {
					makeDirty();
				}
			}
			if (!isDirty()) {
				return false;
			}
		}

		// at this point, this element is dirty
		makeClean();

		// calculate own width and arrange children
		if (layout_style == LayoutStyle::Free) {
			vec2 contentsize = arrangeChildren(size.x);
			size = vec2(
				std::min(std::max({ size.x, contentsize.x, min_size.x }), max_size.x),
				std::min(std::max({ size.y, contentsize.y, min_size.y }), max_size.y)
			);
			display_rect.setSize(size - vec2(2.0f, 2.0f));
			updateChildPositions();
			updatePosition();
			onResize();
			return false;
		} else {
			vec2 newsize = arrangeChildren(width_avail);
			if (layout_style == LayoutStyle::Block) {
				newsize.x = std::max(width_avail, newsize.x);
			}
			size = vec2(
				std::min(std::max(newsize.x, min_size.x), max_size.x),
				std::min(std::max(newsize.y, min_size.y), max_size.y)
			);
			if (size.x > width_avail) {
				newsize = arrangeChildren(size.x);
				size = vec2(
					std::min(std::max(newsize.x, min_size.x), max_size.x),
					std::min(std::max(newsize.y, min_size.y), max_size.y)
				);
			}
			display_rect.setSize(size - vec2(2.0f, 2.0f));
			updateChildPositions();
			onResize();
			vec2 new_total_size = size + vec2(2.0f * margin, 2.0f * margin);
			float diff = abs(old_total_size.x - new_total_size.x) + abs(old_total_size.y - new_total_size.y);
			old_total_size = new_total_size;
			return diff > epsilon;
		}
	}

	struct LayoutData {

		using Child = std::pair<std::shared_ptr<Element>, Element::WhiteSpace>;

		LayoutData(Element& _self, float _width_avail)
			: self(_self),
			width_avail(_width_avail) {

			sorted_elements.reserve(_self.children.size() + _self.white_spaces.size());

			for (const auto& child : _self.children) {
				sorted_elements.push_back({ child, Element::WhiteSpace(Element::WhiteSpace::None, 0.0f) });
			}
			for (const auto& space : _self.white_spaces) {
				sorted_elements.push_back({ nullptr, space });
			}

			reset();
			auto comp = [](const Child& l, const Child& r) {
				return (l.first ? l.first->layout_index : l.second.layout_index) < (r.first ? r.first->layout_index : r.second.layout_index);
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
		std::vector<Child> sorted_elements;
		std::vector<std::shared_ptr<Element>> floatingleft, floatingright;

		void reset() {
			xpos = self.padding;
			ypos = self.padding;
			next_ypos = self.padding;
			contentsize = { 2.0f * self.padding, 2.0f * self.padding };
			left_edge = self.padding;
			right_edge = width_avail - self.padding;
			emptyline = true;
		}

		void layoutElements() {
			std::vector<std::shared_ptr<Element>> left_elems, right_elems;
			std::vector<Child> inline_children;

			auto horizontalAlign = [&, this](const std::vector<std::shared_ptr<Element>>& line, float left_limit, float right_limit, bool full) {
				if (line.size() == 0 || self.content_align == ContentAlign::Left) {
					return;
				}

				if (self.content_align == ContentAlign::Right) {

					float offset = right_limit - (line.back()->pos.x + line.back()->size.x + line.back()->margin);
					for (const auto& elem : line) {
						elem->pos.x = elem->pos.x + offset;
					}

				} else if (self.content_align == ContentAlign::Center) {

					float width = line.back()->pos.x + line.back()->size.x + line.back()->margin - line.front()->pos.x - line.front()->margin;
					float left_wanted = (width_avail - width) * 0.5f;
					float left = std::min(std::max(left_wanted, left_limit), right_limit - width);
					float offset = left - line.front()->pos.x;

					for (const auto& elem : line) {
						elem->pos.x += offset;
					}

				} else if (self.content_align == ContentAlign::Justify) {
					if (line.size() <= 1 || !full) {
						return;
					}

					float offset = (right_limit - (line.back()->pos.x + line.back()->size.x + line.back()->margin)) / (float)(line.size() - 1);
					float acc = 0;
					for (const auto& elem : line) {
						elem->pos.x += acc;
						acc += offset;
					}
				}
			};

			// arranges and empties the current left- and right- floating elements
			// and inline elements, returning the maximum width needed or the current
			// available width if it would be exceeded without breaking onto a new line
			auto layoutBatch = [&, this]() -> float {
				float lwidth = 0.0f;
				float rwidth = 0.0f;
				float iwidth_current = 0.0f;
				float iwidth = 0.0f;
				bool exceeded_width = false;

				std::vector<std::shared_ptr<Element>> line;

				for (const auto& elem : left_elems) {
					if (arrangeFloatingLeft(elem)) {
						exceeded_width = true;
					} else {
						lwidth += elem->size.x + 2.0f * elem->margin;
					}
				}
				for (const auto& elem : right_elems) {
					if (arrangeFloatingRight(elem)) {
						exceeded_width = true;
					} else {
						lwidth += elem->size.x + 2.0f * elem->margin;
					}
				}
				for (const auto& child : inline_children) {
					const std::shared_ptr<Element>& elem = child.first;
					float left = left_edge, right = right_edge;
					if (elem) {
						if (arrangeInline(elem)) {
							horizontalAlign(line, left, right, true);
							line.clear();
							exceeded_width = true;
						} else {
							iwidth_current += elem->size.x + 2.0f * elem->margin;
						}
						line.push_back(elem);
					} else {
						const Element::WhiteSpace& space = child.second;
						if (space.type == Element::WhiteSpace::LineBreak) {
							iwidth = std::max(iwidth, iwidth_current);
							iwidth_current = 0.0f;
							horizontalAlign(line, left, right, false);
							line.clear();
							if (emptyline) {
								next_ypos += (float)space.charsize;
								newLine();
							} else {
								newLine();
							}
						} else if (space.type == Element::WhiteSpace::Tab) {
							iwidth_current += tab(space.charsize, left_edge);
						}
					}
				}
				horizontalAlign(line, left_edge, right_edge, false);
				iwidth = std::max(iwidth, iwidth_current);
				line.clear();
				left_elems.clear();
				right_elems.clear();
				inline_children.clear();
				if (exceeded_width) {
					return this->width_avail;
				} else {
					return lwidth + rwidth + iwidth + 2.0f * self.padding;
				}
			};

			// arrange all elements in batches to fit within the available width,
			// returning the maximum width needed without breaking lines
			// or the available width if it would be exceeded
			auto layoutEverything = [&, this]() -> float {
				float max_width = 0.0f;
				auto it = sorted_elements.cbegin();
				auto end = sorted_elements.cend();
				while (true) {
					if (it == end) {
						max_width = std::max(max_width, layoutBatch());
						break;
					}

					std::shared_ptr<Element> elem = it->first;

					if (elem) {
						// child element
						if (elem->visible) {
							switch (elem->layout_style) {
								case LayoutStyle::Block:
									max_width = std::max(max_width, layoutBatch());
									arrangeBlock(elem);
									break;
								case LayoutStyle::Inline:
									inline_children.push_back({ elem, Element::WhiteSpace(Element::WhiteSpace::None, 0.0f) });
									break;
								case LayoutStyle::FloatLeft:
									left_elems.push_back(elem);
									break;
								case LayoutStyle::FloatRight:
									right_elems.push_back(elem);
									break;
								default:
									elem->update(far_away);
									break;
							}
						}
					} else {
						// line break
						const Element::WhiteSpace& br = it->second;
						inline_children.push_back({ nullptr, br });
					}

					++it;
				}

				return max_width;
			};

			float max_width = layoutEverything();

			bool should_shrink = self.layout_style == LayoutStyle::Inline ||
				self.layout_style == LayoutStyle::FloatLeft ||
				self.layout_style == LayoutStyle::FloatRight;

			if (should_shrink && max_width < width_avail) {
				width_avail = max_width;
				reset();
				layoutEverything();
			}
		}

		void arrangeBlock(const std::shared_ptr<Element>& element) {
			Element& elem = *element;

			while (nextWiderLine()) {

			}

			elem.setPos({ self.padding + elem.margin, next_ypos + elem.margin });
			elem.update(right_edge - left_edge - 2.0f * elem.margin);
			next_ypos = elem.pos.y + elem.size.y + elem.margin;
			fitContents(elem);
			newLine();
		}

		// arranges an inline element adjacent to previous inline elements,
		// flowing around floating elements.
		// returns true if the available width was exceeded and the element
		// broke onto a new line
		bool arrangeInline(const std::shared_ptr<Element>& element) {
			Element& elem = *element;
			bool broke_line = false;
			do {
				// position the element
				elem.setPos({ xpos + elem.margin, ypos + elem.margin });
				elem.update(right_edge - xpos - 2.0f * elem.margin - self.padding);

				if (elem.pos.x + elem.size.x + elem.margin > right_edge) {
					broke_line = true;
					// if it goes past the edge
					if (emptyline) {
						// if it's the only inline element on the line, find the next wider line
						if (!nextWiderLine()) {
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
		bool arrangeFloatingLeft(const std::shared_ptr<Element>& element) {
			if (!emptyline) {
				newLine();
			}
			Element& elem = *element;
			bool broke_line = false;
			do {
				elem.setPos({ left_edge + elem.margin, ypos + elem.margin });
				elem.update(right_edge - left_edge - 2.0f * elem.margin);
				if (elem.pos.x + elem.size.x + elem.padding > right_edge) {
					broke_line = true;
					// if the element doesn't fit
					if (!nextWiderLine()) {
						// if a wider line can't be found
						break;
					}
				} else {
					// if the element fits
					break;
				}
			} while (true);
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
		bool arrangeFloatingRight(const std::shared_ptr<Element>& element) {
			Element& elem = *element;
			bool broke_line = false;
			do {
				float avail = right_edge - left_edge - elem.margin * 2.0f;
				elem.update(avail);
				if (elem.getSize().x > avail) {
					broke_line = true;
					if (!nextWiderLine()) {
						// if a wider line can't be found
						break;
					}
				} else {
					break;
				}
			} while (true);

			elem.setPos({ right_edge - elem.size.x - elem.margin, ypos + elem.margin });
			floatingright.push_back(element);
			right_edge = getRightEdge();
			fitContents(elem);
			return broke_line;
		}

		// adjusts `contentsize` to include the element
		void fitContents(const Element& elem) {
			contentsize = vec2(
				std::max(contentsize.x, elem.pos.x + elem.size.x + elem.margin + self.padding),
				std::max(contentsize.y, elem.pos.y + elem.size.y + elem.margin + self.padding)
			);
		}

		bool nextWiderLine() {
			// if floating left elements, find next bottom edge of floating left and make newline there, return true
			// else return false

			if (floatingleft.size() > 0 || floatingright.size() > 0) {
				float next_y = far_away;
				for (const auto& elem : floatingleft) {
					next_y = std::min(next_y, elem->pos.y + elem->size.y + elem->margin);
				}

				for (const auto& elem : floatingright) {
					next_y = std::min(next_y, elem->pos.y + elem->size.y + elem->margin);
				}

				next_ypos = std::max(next_y, next_ypos);
				newLine();
				return true;
			}

			return false;
		}

		void newLine() {
			ypos = next_ypos;

			auto aboveNewLine = [=](const std::shared_ptr<Element>& elem) {
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

		float tab(unsigned charsize, float left) {
			float tab_size = 50.0f * (float)charsize / 15.0f;
			float nu_xpos = floor(((xpos - left) / tab_size) + 1.0f) * tab_size;
			float diff = nu_xpos - (xpos - left);
			xpos = nu_xpos + left;
			return diff;
		}

		float getLeftEdge() const {
			// Assumption: all elements in `floatingleft` pass through ypos
			float extent = self.padding;
			for (const auto& elem : floatingleft) {
				extent = std::max(extent, elem->getPos().x + elem->getSize().x + elem->getMargin());
			}
			return extent;
		}

		float getRightEdge() const {
			// Assumption: all elements in `floatingright` pass through ypos
			float extent = width_avail - self.padding;
			for (const auto& elem : floatingright) {
				extent = std::min(extent, elem->getPos().x - elem->getMargin());
			}
			return extent;
		}
	};

	vec2 Element::arrangeChildren(float width_avail) {
		if (children.size() == 0 && white_spaces.size() == 0) {
			return { padding, padding };
		}

		LayoutData layout(*this, width_avail);

		layout.layoutElements();

		return layout.contentsize;
	}

	FreeElement::FreeElement() : Element(LayoutStyle::Free) {

	}

	InlineElement::InlineElement() : Element(LayoutStyle::Inline) {

	}

	BlockElement::BlockElement() : Element(LayoutStyle::Block) {

	}

	LeftFloatingElement::LeftFloatingElement() : Element(LayoutStyle::FloatLeft) {

	}

	RightFloatingElement::RightFloatingElement() : Element(LayoutStyle::FloatRight) {

	}

	Element::WhiteSpace::WhiteSpace(Element::WhiteSpace::Type _type, LayoutIndex _layout_index, unsigned _charsize)
		: type(_type), layout_index(_layout_index), charsize(_charsize) {

	}

}