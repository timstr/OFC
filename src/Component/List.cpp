#include <GUI/Component/List.hpp>

namespace ui {

	void List::onMount(const dom::Element* beforeSibling) {
		for (auto& c : m_components) {
			c.tryMount(this, beforeSibling);
		}
	}

	void List::onUnmount() {
		for (auto it = m_components.rbegin(), itEnd = m_components.rend(); it != itEnd; ++it) {
			it->tryUnmount();
		}
	}

	std::vector<const Component*> List::getChildren() const noexcept {
		auto ret = std::vector<const Component*>();
		ret.reserve(m_components.size());
		for (const auto& c : m_components) {
			ret.push_back(c.get());
		}
		return ret;
	}
    
} // namespace ui
