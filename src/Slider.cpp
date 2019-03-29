#include "GUI/Helpers/Slider.hpp"
#include "GUI/Text.hpp"

namespace ui {

	Slider::Slider(float defaultval, float min, float max, const sf::Font& font, std::function<void(float)> _onChange)
		: minimum(std::min(min, max)),
		maximum(std::max(min, max)),
		value(std::min(std::max(defaultval, minimum), maximum)),
		onChange(_onChange) {

		setSize({ 100.0f, 30.0f }, true);
		setBackgroundColor(sf::Color(0xFFFFFFFF));
		label = add<Text>(std::to_wstring(value), font);
		handle = add<Handle>(*this);
	}

	bool Slider::onLeftClick(int) {
		handle->setPos(localMousePos() - handle->size() * 0.5f);
		handle->startDrag();
		return true;
	}

	void Slider::onLeftRelease() {
		handle->stopDrag();
	}

	void Slider::setMinimum(float min) {
		minimum = min;
	}
	float Slider::getMinimum() const {
		return minimum;
	}

	void Slider::setMaximum(float max) {
		maximum = max;
	}
	float Slider::getMaximum() {
		return maximum;
	}

	void Slider::setValue(float val) {
		value = std::min(std::max(minimum, val), maximum);
		label->setText(std::to_string(val));
		moveHandleTo(value);
	}
	float Slider::getValue() const {
		return value;
	}

	StrongRef<Element> Slider::getHandle() const {
		return handle;
	}

	void Slider::onResize() {
		handle->setSize({ height(), height() });
		moveHandleTo(value);
	}

	void Slider::moveHandleTo(float val) {
		float x = (val - minimum) / (maximum - minimum) * (width() - handle->width());
		handle->setPos({ x, 0 });
	}

	bool Slider::onKeyDown(ui::Key key) {
		vec2 delta = (keyDown(ui::Key::LShift) || keyDown(ui::Key::RShift)) ? vec2(0.1f, 0.0f) : vec2(1.0f, 0.0f);
		if (key == ui::Key::Left) {
			handle->setPos(handle->pos() - delta);
			handle->updateFromPos();
			return true;
		}
		if (key == ui::Key::Right) {
			handle->setPos(handle->pos() + delta);
			handle->updateFromPos();
			return true;
		}
		return false;
	}


	Slider::Handle::Handle(Slider& _slider) : slider(_slider), reference_position(0.0f), fine_dragging(false) {
		float s = slider.height();
		setSize({ s, s }, true);
		setBackgroundColor(sf::Color(0x80808080));
		setBorderThickness(0.0f);
	}

	bool Slider::Handle::onLeftClick(int) {
		fine_dragging = false;
		startDrag();
		return true;
	}

	void Slider::Handle::onLeftRelease() {
		stopDrag();
	}

	bool Slider::Handle::onRightClick(int) {
		fine_dragging = true;
		reference_position = left();
		startDrag();
		return true;
	}

	void Slider::Handle::onRightRelease() {
		stopDrag();
	}

	void Slider::Handle::onDrag() {
		if (fine_dragging) {
			float diff = left() - reference_position;
			setLeft(reference_position + fine_speed * diff);
		}
		updateFromPos();
	}

	bool Slider::Handle::onKeyDown(ui::Key key) {
		vec2 delta = (keyDown(ui::Key::LShift) || keyDown(ui::Key::RShift)) ? vec2(0.1f, 0.0f) : vec2(1.0f, 0.0f);
		if (key == ui::Key::Left) {
			setPos(pos() - delta);
			updateFromPos();
			return true;
		}
		if (key == ui::Key::Right) {
			setPos(pos() + delta);
			updateFromPos();
			return true;
		}
		return false;
	}

	void Slider::Handle::updateFromPos() {
		vec2 p = pos();
		float left = 0.0f;
		float right = slider.width() - width();
		p.x = std::min(std::max(p.x, left), right);
		p.y = 0.0f;
		setPos(p);
		float x = (p.x - left) / (right - left);
		slider.value = x * (slider.maximum - slider.minimum) + slider.minimum;
		slider.label->setText(std::to_string(slider.value));
		slider.onChange(slider.value);
	}

	const float Slider::Handle::fine_speed = 0.1f;

} // namespace ui