#include "GUI/Element.hpp"
#include "GUI/GUI.hpp"
#include "GUI/Text.hpp"
#include "GUI/RoundedRectangle.hpp"

#include <algorithm>
#include <set>
#include <sstream>

namespace ui {
	namespace {
		const float epsilon = 0.0001f;
		const float far_away = 1000000.0f;
	}

	Element::Element(LayoutStyle _display_style) noexcept :
		m_temp_this(this, [](Element* e){ delete e; }),
		m_closed(false),
		m_layoutstyle(_display_style),
		m_contentalign(ContentAlign::Left),
		m_pos({ 0.0f, 0.0f }),
		m_size({ 100.0f, 100.0f }),
		m_minsize({ 0.0f, 0.0f }),
		m_maxsize({ far_away, far_away }),
		m_oldtotalsize({ 0.0f, 0.0f }),
		m_disabled(false),
		m_visible(true),
		m_clipping(false),
		m_keyboard_navigable(true),
		m_isdirty(true),
		m_layoutindex(0.0f),
		m_padding(0.0f),
		m_margin(0.0f),
		m_pstyle_x(PositionStyle::None),
		m_pstyle_y(PositionStyle::None),
		m_spacing_x(0.0f),
		m_spacing_y(0.0f) {

		m_displayrect.setSize(size());
		m_displayrect.setFillColor(sf::Color(0));
		m_displayrect.setOutlineThickness(0.0f);
		m_displayrect.setOutlineColor(sf::Color(0xFF));
	}

	Element& Element::disable() noexcept {
		m_disabled = true;
		return *this;
	}

	Element& Element::enable() noexcept {
		m_disabled = false;
		return *this;
	}

	bool Element::isEnabled() const noexcept {
		return !m_disabled;
	}

	Element& Element::enableKeyboardNavigation() noexcept {
		m_keyboard_navigable = true;
		return *this;
	}

	Element& Element::disableKeyboardNavigation() noexcept {
		m_keyboard_navigable = false;
		return *this;
	}

	bool Element::keyboardNavigable() const noexcept {
		return m_keyboard_navigable;
	}

	Element& Element::setVisible(bool is_visible) noexcept {
		if (isVisible() != is_visible && layoutStyle() != LayoutStyle::Free) {
			makeDirty();
		}
		m_visible = is_visible;
		return *this;
	}

	bool Element::isVisible() const noexcept {
		return m_visible;
	}

	Element& Element::setClipping(bool _clipping) noexcept {
		m_clipping = _clipping;
		return *this;
	}

	bool Element::clipping() const noexcept {
		return m_clipping;
	}

	vec2 Element::pos() const noexcept {
		return m_pos;
	}

	float Element::left() const noexcept {
		return m_pos.x;
	}

	float Element::top() const noexcept {
		return m_pos.y;
	}

	Element& Element::setPos(vec2 _pos) noexcept {
		if (abs(left() - _pos.x) + abs(top() - _pos.y) > epsilon) {
			m_pos = _pos;
		}
		if (m_layoutstyle == LayoutStyle::Free) {
			updatePosition();
		}
		return *this;
	}

	Element & Element::setLeft(float x) noexcept {
		return setPos({ x, top() });
	}

	Element & Element::setTop(float y) noexcept {
		return setPos({ left(), y });
	}

	vec2 Element::size() const noexcept {
		return m_size;
	}

	Element& Element::setSize(vec2 _size, bool force) noexcept {
		_size = vec2(std::max(_size.x, 0.0f), std::max(_size.y, 0.0f));
		if (abs(width() - _size.x) + abs(height() - _size.y) > epsilon) {
			m_size = _size;
			makeDirty();
		}
		if (force) {
			if (minWidth() > width() || minHeight() > height() || maxWidth() < width() || maxHeight() < height()) {
				makeDirty();
			}
			m_minsize = _size;
			m_maxsize = _size;
		}
		return *this;
	}

	Element& Element::setMinSize(vec2 _min_size) noexcept {
		_min_size = vec2(std::max(_min_size.x, 0.0f), std::max(_min_size.y, 0.0f));
		m_minsize = _min_size;
		makeDirty();
		return *this;
	}

	vec2 Element::maxSize() const noexcept {
		return m_maxsize;
	}

	vec2 Element::minSize() const noexcept {
		return m_minsize;
	}

	Element& Element::setMinWidth(float width) noexcept {
		return setMinSize({ width, minHeight() });
	}

	Element& Element::setMinHeight(float height) noexcept {
		return setMinSize({ minWidth(), height });
	}

	Element& Element::setMaxSize(vec2 _max_size) noexcept {
		_max_size = vec2(std::max(_max_size.x, 0.0f), std::max(_max_size.y, 0.0f));
		m_maxsize = _max_size;
		makeDirty();
		return *this;
	}

	Element& Element::setMaxWidth(float width) noexcept {
		return setMaxSize({ width, maxHeight() });
	}

	Element& Element::setWidth(float _width, bool force) noexcept {
		_width = std::max(0.0f, _width);
		if (abs(width() - _width) > epsilon) {
			m_size.x = _width;
			makeDirty();
		}
		if (force) {
			if (width() < minWidth() || width() > maxWidth()) {
				makeDirty();
			}
			m_minsize.x = _width;
			m_maxsize.x = _width;
		}
		return *this;
	}

	float Element::minWidth() const noexcept {
		return m_minsize.x;
	}

	float Element::maxWidth() const noexcept {
		return m_maxsize.x;
	}

	float Element::width() const noexcept {
		return m_size.x;
	}

	Element& Element::setMaxHeight(float height) noexcept {
		return setMaxSize({ maxWidth(), height });
	}

	Element& Element::setHeight(float _height, bool force) noexcept {
		_height = std::max(0.0f, _height);
		if (abs(height() - _height) > epsilon) {
			m_size.y = _height;
			makeDirty();
		}
		if (force) {
			if (height() < minHeight() || height() > maxHeight()) {
				makeDirty();
			}
			m_minsize.y = _height;
			m_maxsize.y = _height;
		}
		return *this;
	}

	float Element::maxHeight() const noexcept {
		return m_maxsize.y;
	}

	float Element::minHeight() const noexcept {
		return m_minsize.y;
	}

	float Element::height() const noexcept {
		return m_size.y;
	}

	void Element::onResize() {

	}

	void Element::setLayoutStyle(LayoutStyle style) noexcept {
		if (layoutStyle() != style) {
			m_layoutstyle = style;
			if (auto par = parent().lock()) {
				par->makeDirty();
			}
		}
	}

	LayoutStyle Element::layoutStyle() const noexcept {
		return m_layoutstyle;
	}

	Element& Element::setContentAlign(ContentAlign style) noexcept {
		if (contentAlign() != style) {
			m_contentalign = style;
			makeDirty();
		}
		return *this;
	}

	ContentAlign Element::contentAlign() const noexcept {
		return m_contentalign;
	}

	void Element::setXPositionStyle(PositionStyle style, float spacing) noexcept {
		m_pstyle_x = style;
		m_spacing_x = spacing;
	}

	PositionStyle Element::xPositionStyle() const noexcept {
		return m_pstyle_x;
	}

	float Element::xPositionSpacing() const noexcept {
		return m_spacing_x;
	}

	void Element::setYPositionStyle(PositionStyle style, float spacing) noexcept {
		m_pstyle_y = style;
		m_spacing_y = spacing;
	}

	PositionStyle Element::yPositionStyle() const noexcept {
		return m_pstyle_y;
	}

	float Element::yPositionSpacing() const noexcept {
		return m_spacing_y;
	}

	Element::~Element() {
		close();
	}

	void Element::close() noexcept {
		if (isClosed()) {
			return;
		}
		m_closed = true;
		onClose();
		while (!m_children.empty()) {
			if (m_children.back()->ancestorInFocus()) {
				grabFocus();
			}
			m_children.back()->close();
		}
		auto self = shared_from_this();
		if (auto p = parent().lock()) {
			p->remove(self);
		}
	}

	bool Element::isClosed() const noexcept {
		return m_closed;
	}

	void Element::onClose() {

	}

	bool Element::hit(vec2 testpos) const {
		return ((testpos.x >= 0.0f) && (testpos.x < width()) && (testpos.y >= 0.0f) && (testpos.y < height()));
	}

	vec2 Element::localMousePos() const noexcept {
		vec2 mousepos = (vec2)sf::Mouse::getPosition(getContext().getRenderWindow());
		Ref<const Element> element = shared_from_this();
		while (element) {
			mousepos -= element->pos();
			element = element->parent().lock();
		}
		return mousepos;
	}

	vec2 Element::absPos() const noexcept {
		vec2 rootpos = { 0, 0 };
		Ref<const Element> element = shared_from_this();
		while (element) {
			rootpos += element->pos();
			element = element->parent().lock();
		}
		return rootpos;
	}

	bool Element::onLeftClick(int) {
		return false;
	}

	void Element::onLeftRelease() {

	}

	bool Element::onRightClick(int) {
		return false;
	}

	void Element::onRightRelease() {

	}

	bool Element::onMiddleClick(int clicks) {
		return false;
	}

	void Element::onMiddleRelease() {

	}

	bool Element::leftMouseDown() const noexcept {
		return sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
	}

	bool Element::rightMouseDown() const noexcept {
		return sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
	}

	bool Element::middleMouseDown() const noexcept {
		return sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle);
	}

	bool Element::onScroll(float, float) {
		return false;
	}

	void Element::startDrag() noexcept {
		if (layoutStyle() == LayoutStyle::Free) {
			grabFocus();
			getContext().setDraggingElement(shared_from_this(), (vec2)sf::Mouse::getPosition(getContext().getRenderWindow()) - pos());
		}
	}

	void Element::onDrag() {

	}

	void Element::stopDrag() noexcept {
		if (dragging()) {
			getContext().setDraggingElement(nullptr);
		}
	}

	bool Element::dragging() const noexcept {
		return (getContext().getDraggingElement().get() == this);
	}

	void Element::onMouseOver() {

	}

	void Element::onMouseOut() {

	}

	bool Element::hovering() const noexcept {
		return getContext().getHoverElement().get() == this;
	}

	bool Element::onHover() {
		return false;
	}

	bool Element::onHoverWith(const Ref<Element>& element) {
		return false;
	}

	void Element::drop(vec2 local_pos) noexcept {
		vec2 drop_pos = absPos() + local_pos;
		auto self = shared_from_this();
		if (auto element = root().findElementAt(drop_pos, this)) {
			do {
				if (element->onDrop(self)) {
					return;
				}
			} while (element = element->parent().lock());
		}
	}

	bool Element::onDrop(const Ref<Element>& element) {
		return false;
	}

	void Element::onFocus() {

	}

	bool Element::inFocus() const noexcept {
		return (getContext().getCurrentElement().get() == this);
	}

	void Element::onLoseFocus() {

	}

	void Element::grabFocus() noexcept {
		getContext().focusTo(shared_from_this());
	}

	bool Element::onKeyDown(Key) {
		return false;
	}

	void Element::onKeyUp(Key) {

	}

	bool Element::keyDown(Key key) const noexcept {
		return sf::Keyboard::isKeyPressed(key);
	}

	void Element::write(const std::string& text, const sf::Font& font, sf::Color color, unsigned charsize, TextStyle style) noexcept {
		write(std::wstring { text.begin(), text.end() }, font, color, charsize, style);
	}

	void Element::write(const std::wstring& text, const sf::Font& font, sf::Color color, unsigned charsize, TextStyle style) noexcept {
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

	void Element::writeLineBreak(unsigned charsize) noexcept {
		m_whitespaces.push_back(WhiteSpace(WhiteSpace::LineBreak, getNextLayoutIndex(), charsize));
		makeDirty();
	}

	void Element::writePageBreak(float height) noexcept {
		auto br = add<BlockElement>();
		br->setMargin(height * 0.5f);
		br->setBorderColor(sf::Color(0));
		br->setBackgroundColor(sf::Color(0));
		br->disable();
		br->setSize({ 0.0f, 0.0f });
	}

	void Element::writeTab(float width) noexcept {
		unsigned charsize = static_cast<unsigned>(floor(width / 50.0f * 15.0f));
		m_whitespaces.push_back(WhiteSpace(WhiteSpace::Tab, getNextLayoutIndex(), charsize));
		makeDirty();
	}

	void Element::adopt(Ref<Element> child) noexcept {
		if (auto p = child->parent().lock()) {
			if (p.get() == this) {
				return;
			}
			p->release(child);
		}
		m_children.push_back(child);
		child->m_parent = weak_from_this();
		child->m_layoutindex = getNextLayoutIndex();
		makeDirty();
		if (layoutStyle() != LayoutStyle::Free){
			if (auto p = parent().lock()) {
				p->makeDirty();
			}
		}
	}

	void Element::remove(const Ref<Element>& element) noexcept {
		if (element) {
			for (auto it = m_children.begin(); it != m_children.end(); ++it) {
				if (*it == element) {
					if (element->ancestorInFocus()) {
						grabFocus();
					}
					m_children.erase(it);
					element->m_parent = {};
					element->close();
					organizeLayoutIndices();
					makeDirty();
					return;
				}
			}
		}
	}

	Ref<Element> Element::release(const Ref<Element>& element) noexcept {
		if (element) {
			for (auto it = m_children.begin(); it != m_children.end(); ++it) {
				if (*it == element) {
					if (element->ancestorInFocus()) {
						grabFocus();
					}
					m_children.erase(it);
					element->m_parent = {};
					organizeLayoutIndices();
					makeDirty();
					return element;
				}
			}
		}
		return nullptr;
	}

	bool Element::has(const Ref<Element>& child) const noexcept {
		return child->parent().lock().get() == this;
	}

	void Element::bringToFront() noexcept {
		if (auto p = parent().lock()) {
			auto self = shared_from_this();
			auto& c = p->m_children;
			c.erase(std::remove_if(c.begin(), c.end(), [self](const Ref<Element>& e){ return e == self; }), c.end());
			c.push_back(self);
		}
	}

	void Element::clear() noexcept {
		while (m_children.size() > 0) {
			m_children.back()->close();
		}
		m_children.clear();
		makeDirty();
	}

	Ref<Element> Element::findElementAt(vec2 _pos, const Element* exclude){
		if (!isVisible() || !isEnabled()) {
			return nullptr;
		}

		if (clipping() && ((_pos.x < 0.0f) || (_pos.x > width()) || (_pos.y < 0.0) || (_pos.y > height()))) {
			return nullptr;
		}

		if (exclude == this) {
			return nullptr;
		}

		Ref<Element> element;
		for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
			element = (*it)->findElementAt(_pos - (*it)->pos(), exclude);
			if (element) {
				return element;
			}
		}

		if (this->hit(_pos)) {
			return shared_from_this();
		}

		return nullptr;
	}

	void Element::render(sf::RenderWindow& rw) {
		rw.draw(m_displayrect);
	}

	void* Element::operator new(size_t size) {
		return ::operator new(size);
	}

	void Element::operator delete(void* ptr) {
		::operator delete(ptr);
	}

	bool Element::navigateToPreviousElement(){
		if (!inFocus()) {
			return false;
		}
		auto par = parent().lock();
		if (!par) {
			return false;
		}
		std::map<LayoutIndex, Ref<Element>> siblings;
		for (const auto& sib : par->children()) {
			if (sib->isEnabled() && sib->isVisible()) {
				siblings[sib->m_layoutindex] = sib;
			}
		}
		if (siblings.size() < 2) {
			return par->navigateToPreviousElement();
		}

		auto self = siblings.find(this->m_layoutindex);

		if (self == siblings.begin()) {
			self = siblings.end();
		}
		--self;
		self->second->grabFocus();
		getContext().highlightCurrentElement();
		return true;
	}

	bool Element::navigateToNextElement(){
		if (!inFocus()) {
			return false;
		}
		auto par = parent().lock();
		if (!par) {
			return false;
		}
		std::map<LayoutIndex, Ref<Element>> siblings;
		for (const auto& sib : par->children()) {
			if (sib->isEnabled() && sib->isVisible()) {
				siblings[sib->m_layoutindex] = sib;
			}
		}
		if (siblings.size() < 2) {
			return par->navigateToNextElement();
		}

		auto self = siblings.find(this->m_layoutindex);

		++self;
		if (self == siblings.end()) {
			self = siblings.begin();
		}
		self->second->grabFocus();
		getContext().highlightCurrentElement();
		return true;
	}

	bool Element::navigateIn(){
		if (!inFocus()) {
			return false;
		}
		std::map<LayoutIndex, Ref<Element>> elems;
		for (const auto& child : children()) {
			if (child->isEnabled() && child->isVisible()) {
				elems[child->m_layoutindex] = child;
			}
		}
		if (elems.size() == 0) {
			if (!inFocus()) {
				grabFocus();
				getContext().highlightCurrentElement();
			}
			return true;
		} else if (elems.size() == 1) {
			return elems.begin()->second->navigateIn();
		} else if (elems.size() > 1) {
			elems.begin()->second->grabFocus();
			getContext().highlightCurrentElement();
			return true;
		}
		return false;
	}

	bool Element::navigateOut(){
		if (!inFocus()) {
			return false;
		}
		auto par = parent().lock();
		if (!par) {
			return false;
		}
		int count = 0;
		for (const auto& sib : par->children()) {
			if (sib->isEnabled() && sib->isVisible()) {
				++count;
			}
		}
		if (count >= 2) {
			par->grabFocus();
			getContext().highlightCurrentElement();
			return true;
		} else {
			return par->navigateOut();
		}
	}

	void Element::renderChildren(sf::RenderWindow& renderwindow){
		// save view state
		const vec2 offset = getContext().getViewOffset();
		const sf::FloatRect cliprect = getContext().getClipRect();
		for (const Ref<Element>& child : m_children){
			if (child->isVisible() && child) {
				if (child->clipping()) {
					auto childrect = sf::FloatRect(-offset + child->pos(), child->size());
					if (!getContext().getClipRect().intersects(childrect)) {
						// restore previous view state
						getContext().setViewOffset(offset);
						getContext().setClipRect(cliprect);
						getContext().updateView();
						continue;
					}
					getContext().setViewOffset(offset - child->pos());
					sf::FloatRect rect = getContext().getClipRect();
					getContext().intersectClipRect(sf::FloatRect(-offset, child->size()));
					getContext().updateView();
					child->render(renderwindow);
					child->renderChildren(renderwindow);
				} else {
					getContext().setViewOffset(offset - child->pos());
					getContext().updateView();
					child->render(renderwindow);
					child->renderChildren(renderwindow);
				}
				// restore previous view state
				getContext().setViewOffset(offset);
				getContext().setClipRect(cliprect);
				getContext().updateView();
			}
		}
	}

	Element::LayoutIndex Element::getNextLayoutIndex() const noexcept {
		LayoutIndex max = 0.0f;
		for (const auto& child : m_children) {
			max = std::max(child->m_layoutindex, max);
		}
		for (const auto& space : m_whitespaces) {
			max = std::max(space.layout_index, max);
		}
		return max + 1.0f;
	}

	void Element::organizeLayoutIndices() noexcept {
		using ElementOrSpace = std::pair<Ref<Element>, WhiteSpace*>;

		std::set<std::pair<LayoutIndex, ElementOrSpace>> index_set;
		for (const auto& child : m_children) {
			index_set.insert({ child->m_layoutindex, {child, nullptr} });
		}
		for (auto& space : m_whitespaces) {
			index_set.insert({ space.layout_index, {nullptr, &space} });
		}

		LayoutIndex i = 0.0f;
		for (const auto& mapping : index_set) {
			if (mapping.second.first) {
				mapping.second.first->m_layoutindex = i;
			} else if (mapping.second.second) {
				mapping.second.second->layout_index = i;
			}
			i += 1.0f;
		}
	}

	bool Element::ancestorInFocus() const noexcept {
		auto elem = getContext().getCurrentElement();
		while (elem) {
			if (elem.get() == this) {
				return true;
			}
			elem = elem->parent().lock();
		}
		return false;
	}

	std::vector<std::pair<Ref<Element>, Element::WhiteSpace>> Element::sortChildrenByLayoutIndex() const noexcept {
		std::vector<std::pair<Ref<Element>, Element::WhiteSpace>> sorted_elements;

		sorted_elements.reserve(m_children.size() + m_whitespaces.size());

		for (const auto& child : m_children) {
			sorted_elements.push_back({ child, Element::WhiteSpace(Element::WhiteSpace::None, 0.0f) });
		}
		for (const auto& space : m_whitespaces) {
			sorted_elements.push_back({ nullptr, space });
		}

		return sorted_elements;
	}

	const std::vector<Ref<Element>>& Element::children() const noexcept {
		return m_children;
	}

	std::weak_ptr<Element> Element::parent() const noexcept {
		return m_parent;
	}

	void Element::layoutBefore(const Ref<Element>& sibling) noexcept {
		if (!sibling || isClosed()) {
			return;
		}
		if (auto mypar = parent().lock()) {
			if (auto otherpar = sibling->parent().lock()) {
				if (mypar != otherpar) {
					return;
				}
				m_layoutindex = sibling->m_layoutindex - 0.5f;
				organizeLayoutIndices();
			}
		}
	}

	void Element::layoutAfter(const Ref<Element>& sibling) noexcept {
		if (!sibling || isClosed()) {
			return;
		}
		if (auto mypar = parent().lock()) {
			if (auto otherpar = sibling->parent().lock()) {
				if (mypar != otherpar) {
					return;
				}
				m_layoutindex = sibling->m_layoutindex + 0.5f;
				organizeLayoutIndices();
			}
		}
	}

	void Element::setPadding(float _padding) noexcept {
		if (abs(padding() - _padding) > epsilon) {
			m_padding = std::max(_padding, 0.0f);
			makeDirty();
		}
	}

	float Element::padding() const noexcept {
		return m_padding;
	}

	void Element::setMargin(float _margin) noexcept {
		_margin = std::max(_margin, 0.0f);
		if (abs(margin() - _margin) > epsilon) {
			m_margin = _margin;
			makeDirty();
		}
	}

	float Element::margin() const noexcept {
		return m_margin;
	}

	sf::Color Element::backgroundColor() const noexcept {
		return m_displayrect.getFillColor();
	}

	void Element::setBackgroundColor(sf::Color color) noexcept {
		m_displayrect.setFillColor(color);
	}

	sf::Color Element::borderColor() const noexcept {
		return m_displayrect.getOutlineColor();
	}

	void Element::setBorderColor(sf::Color color) noexcept {
		m_displayrect.setOutlineColor(color);
	}

	float Element::borderRadius() const noexcept {
		return m_displayrect.getRadius();
	}

	void Element::setBorderRadius(float radius) noexcept {
		m_displayrect.setRadius(radius);
	}

	float Element::borderThickness() const noexcept {
		return m_displayrect.getOutlineThickness();
	}

	void Element::setBorderThickness(float thickness) noexcept {
		m_displayrect.setOutlineThickness(std::max(0.0f, thickness));
	}

	void Element::updatePosition() noexcept {
		if (layoutStyle() != LayoutStyle::Free) {
			return;
		}
		auto par = parent().lock();
		if (!par) {
			return;
		}
		switch (xPositionStyle()) {
			case PositionStyle::OutsideBegin:
				m_pos.x = -xPositionSpacing() - width();
				break;
			case PositionStyle::InsideBegin:
				m_pos.x = xPositionSpacing();
				break;
			case PositionStyle::Center:
				m_pos.x = (par->width() - width()) * 0.5f;
				break;
			case PositionStyle::InsideEnd:
				m_pos.x = par->width() - width() - xPositionSpacing();
				break;
			case PositionStyle::OutsideEnd:
				m_pos.x = par->width() + xPositionSpacing();
				break;
		}
		switch (yPositionStyle()) {
			case PositionStyle::OutsideBegin:
				m_pos.y = -yPositionSpacing() - height();
				break;
			case PositionStyle::InsideBegin:
				m_pos.y = yPositionSpacing();
				break;
			case PositionStyle::Center:
				m_pos.y = (par->height() - height()) * 0.5f;
				break;
			case PositionStyle::InsideEnd:
				m_pos.y = par->height() - height() - yPositionSpacing();
				break;
			case PositionStyle::OutsideEnd:
				m_pos.y = par->height() + yPositionSpacing();
				break;
		}
	}

	void Element::updateChildPositions() noexcept {
		for (const auto& child : m_children) {
			child->updatePosition();
		}
	}

	void Element::makeDirty() noexcept {
		m_isdirty = true;
	}

	bool Element::isDirty() const noexcept {
		return m_isdirty;
	}

	void Element::makeClean() noexcept {
		m_isdirty = false;
	}

	bool Element::update(float width_avail){
		if (this->layoutStyle() == LayoutStyle::Free) {
			width_avail = width();
		}
		width_avail = std::min(std::max(width_avail, minWidth()), maxWidth());

		if (layoutStyle() == LayoutStyle::Block) {
			setWidth(width_avail);
		}

		if (!isDirty()) {
			for (auto child = m_children.begin(); !isDirty() && child != m_children.end(); ++child) {
				if ((*child)->update((*child)->width())) {
					makeDirty();
				}
			}
			if (!isDirty()) {
				return false;
			}
		}

		// calculate own width and arrange children
		vec2 newsize = arrangeChildren(width_avail);
		if (layoutStyle() == LayoutStyle::Block) {
			newsize.x = std::max(width_avail, newsize.x);
		}
		m_size = vec2(
			std::min(std::max(newsize.x, minWidth()), maxWidth()),
			std::min(std::max(newsize.y, minHeight()), maxHeight())
		);
		if (width() > width_avail) {
			newsize = arrangeChildren(width());
			m_size = vec2(
				std::min(std::max(newsize.x, minWidth()), maxWidth()),
				std::min(std::max(newsize.y, minHeight()), maxHeight())
			);
		}
		m_displayrect.setSize(size());
		updatePosition();
		updateChildPositions();
		makeClean();

		onResize();

		if (this->layoutStyle() == LayoutStyle::Free) {
			return false;
		} else {
			vec2 new_total_size = size() + vec2(2.0f * margin(), 2.0f * margin());
			float diff = abs(m_oldtotalsize.x - new_total_size.x) + abs(m_oldtotalsize.y - new_total_size.y);
			m_oldtotalsize = new_total_size;
			return diff > epsilon;
		}
	}

	struct LayoutData {

		using Child = std::pair<Ref<Element>, Element::WhiteSpace>;

		LayoutData(Element& _self, float _width_avail)
			: self(_self),
			width_avail(_width_avail) {

			sorted_elements = self.sortChildrenByLayoutIndex();

			reset();
			auto comp = [](const Child& l, const Child& r) {
				return (l.first ? l.first->m_layoutindex : l.second.layout_index) < (r.first ? r.first->m_layoutindex : r.second.layout_index);
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
		std::vector<Ref<Element>> floatingleft, floatingright;

		void reset() {
			xpos = self.padding();
			ypos = self.padding();
			next_ypos = self.padding();
			contentsize = { 2.0f * self.padding(), 2.0f * self.padding() };
			left_edge = self.padding();
			right_edge = width_avail - self.padding();
			emptyline = true;
		}

		void layoutElements() {
			std::vector<Ref<Element>> left_elems, right_elems;
			std::vector<Child> inline_children;

			auto horizontalAlign = [&, this](const std::vector<Ref<Element>>& line, float left_limit, float right_limit, bool full) {
				if (line.size() == 0 || self.contentAlign() == ContentAlign::Left) {
					return;
				}

				if (self.contentAlign() == ContentAlign::Right) {

					float offset = right_limit - (line.back()->left() + line.back()->width() + line.back()->margin());
					for (const auto& elem : line) {
						elem->m_pos.x += offset;
					}

				} else if (self.contentAlign() == ContentAlign::Center) {

					float width = line.back()->left() + line.back()->width() + line.back()->margin() - line.front()->left() - line.front()->margin();
					float left_wanted = (width_avail - width) * 0.5f;
					float left = std::min(std::max(left_wanted, left_limit), right_limit - width);
					float offset = left - line.front()->left();

					for (const auto& elem : line) {
						elem->m_pos.x += offset;
					}

				} else if (self.contentAlign() == ContentAlign::Justify) {
					if (line.size() <= 1 || !full) {
						return;
					}

					float offset = (right_limit - (line.back()->left() + line.back()->width() + line.back()->margin())) / (float)(line.size() - 1);
					float acc = 0;
					for (const auto& elem : line) {
						elem->m_pos.x += acc;
						acc += offset;
					}
				}
			};

			// arranges and empties the current left- and right- floating elements
			// and inline elements, returning the maximum width needed or the current
			// available width if it would be exceeded without breaking onto a new line
			auto layoutBatch = [&, this]() -> float {

				// maximum natural size of left floating elements
				float lwidth = 0.0f;

				// maximum natural size of right floating elements
				float rwidth = 0.0f;

				// current size of inline elements
				float iwidth_current = 0.0f;

				// maximum size of inline elements
				float iwidth = 0.0f;

				// true if the available size is exceeded
				bool exceeded_width = false;

				// width of the largest element
				float largest_width = 0.0f;

				std::vector<Ref<Element>> line;

				for (const auto& elem : left_elems) {
					if (arrangeFloatingLeft(elem)) {
						exceeded_width = true;
					} else {
						lwidth += elem->width() + 2.0f * elem->margin();
					}
					largest_width = std::max(largest_width, elem->width());
				}
				for (const auto& elem : right_elems) {
					if (arrangeFloatingRight(elem)) {
						exceeded_width = true;
					} else {
						lwidth += elem->width() + 2.0f * elem->margin();
					}
					largest_width = std::max(largest_width, elem->width());
				}
				for (const auto& child : inline_children) {
					const Ref<Element>& elem = child.first;
					float left = left_edge, right = right_edge;
					if (elem) {
						if (arrangeInline(elem)) {
							horizontalAlign(line, left, right, true);
							line.clear();
							exceeded_width = true;
						} else {
							iwidth_current += elem->width() + 2.0f * elem->margin();
						}
						line.push_back(elem);
						largest_width = std::max(largest_width, elem->width());
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
					return std::max(this->width_avail, largest_width);
				} else {
					return lwidth + rwidth + iwidth + 2.0f * self.padding();
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

					Ref<Element> elem = it->first;

					if (elem) {
						// child element
						if (elem->isVisible()) {
							switch (elem->layoutStyle()) {
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

			bool should_shrink = self.layoutStyle() == LayoutStyle::Inline ||
				self.layoutStyle() == LayoutStyle::FloatLeft ||
				self.layoutStyle() == LayoutStyle::FloatRight;

			if ((should_shrink && max_width < width_avail) || (max_width > width_avail && (max_width <= self.maxWidth()))) {
				width_avail = max_width;
				reset();
				layoutEverything();
			}
		}

		void arrangeBlock(const Ref<Element>& element) {
			Element& elem = *element;

			while (nextWiderLine()) {

			}

			elem.setPos({ self.padding() + elem.margin(), next_ypos + elem.margin() });
			elem.update(right_edge - left_edge - 2.0f * elem.margin());
			next_ypos = elem.top() + elem.height() + elem.margin();
			fitContents(elem);
			newLine();
		}

		// arranges an inline element adjacent to previous inline elements,
		// flowing around floating elements.
		// returns true if the available width was exceeded and the element
		// broke onto a new line
		bool arrangeInline(const Ref<Element>& element) {
			Element& elem = *element;
			bool broke_line = false;
			do {
				// position the element
				elem.setPos({ xpos + elem.margin(), ypos + elem.margin() });
				elem.update(right_edge - xpos - 2.0f * elem.margin() - self.padding());

				if (elem.left() + elem.width() + elem.margin() > right_edge) {
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

			xpos = elem.left() + elem.width() + elem.margin();
			next_ypos = std::max(next_ypos, elem.top() + elem.height() + elem.margin());
			fitContents(elem);
			emptyline = false;
			return broke_line;
		}

		// arranges a left-floating element to the right of the left edge
		// and any current left-floating elements.
		// returns true if the available width was exceeded and the element
		// broke onto a new line
		bool arrangeFloatingLeft(const Ref<Element>& element) {
			if (!emptyline) {
				newLine();
			}
			Element& elem = *element;
			bool broke_line = false;
			do {
				elem.setPos({ left_edge + elem.margin(), ypos + elem.margin() });
				elem.update(right_edge - left_edge - 2.0f * elem.margin());
				if (elem.left() + elem.width() + elem.padding() > right_edge) {
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
		bool arrangeFloatingRight(const Ref<Element>& element) {
			Element& elem = *element;
			bool broke_line = false;
			do {
				float avail = right_edge - left_edge - elem.margin() * 2.0f;
				elem.update(avail);
				if (elem.width() > avail) {
					broke_line = true;
					if (!nextWiderLine()) {
						// if a wider line can't be found
						break;
					}
				} else {
					break;
				}
			} while (true);

			elem.setPos({ right_edge - elem.width() - elem.margin(), ypos + elem.margin() });
			floatingright.push_back(element);
			right_edge = getRightEdge();
			fitContents(elem);
			return broke_line;
		}

		// adjusts `contentsize` to include the element
		void fitContents(const Element& elem) {
			contentsize = vec2(
				std::max(contentsize.x, elem.left() + elem.width() + elem.margin() + self.padding()),
				std::max(contentsize.y, elem.top() + elem.height() + elem.margin() + self.padding())
			);
		}

		bool nextWiderLine() {
			// if floating left elements, find next bottom edge of floating left and make newline there, return true
			// else return false

			if (floatingleft.size() > 0 || floatingright.size() > 0) {
				float next_y = far_away;
				for (const auto& elem : floatingleft) {
					next_y = std::min(next_y, elem->top() + elem->height() + elem->margin());
				}

				for (const auto& elem : floatingright) {
					next_y = std::min(next_y, elem->top() + elem->height() + elem->margin());
				}

				next_ypos = std::max(next_y, next_ypos);
				newLine();
				return true;
			}

			return false;
		}

		void newLine() {
			ypos = next_ypos;

			auto aboveNewLine = [=](const Ref<Element>& elem) {
				return ypos >= elem->top() + elem->height() + elem->margin();
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
			float extent = self.padding();
			for (const auto& elem : floatingleft) {
				extent = std::max(extent, elem->left() + elem->width() + elem->margin());
			}
			return extent;
		}

		float getRightEdge() const {
			// Assumption: all elements in `floatingright` pass through ypos
			float extent = width_avail - self.padding();
			for (const auto& elem : floatingright) {
				extent = std::min(extent, elem->left() - elem->margin());
			}
			return extent;
		}
	};

	vec2 Element::arrangeChildren(float width_avail){
		if (m_children.size() == 0 && m_whitespaces.size() == 0) {
			return { padding(), padding() };
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

	Element::WhiteSpace::WhiteSpace(Element::WhiteSpace::Type _type, LayoutIndex _layout_index, unsigned _charsize) noexcept
		: type(_type), layout_index(_layout_index), charsize(_charsize) {

	}

}