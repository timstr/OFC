#pragma once

#include <GUI/Control.hpp>
#include <GUI/FlowContainer.hpp>
#include <GUI/BoxElement.hpp>
#include <GUI/Text.hpp>

namespace ui {

    class CallbackButton : public Control, public Boxed<FlowContainer> {
	private:
		enum class State {
			Normal,
			Hover,
			Active
		};

    public:
        CallbackButton(const String& label, const sf::Font& font, std::function<void()> onClick = {});

		void setNormalColor(Color);
		Color getNormalColor() const;
		void setHoverColor(Color);
		Color getHoverColor() const;
		void setActiveColor(Color);
		Color getActiveColor() const;

		void setCallback(std::function<void()> callback);

		Text& getCaption() noexcept;
		const Text& getCaption() const noexcept;

		bool onLeftClick(int) override;
		void onLeftRelease() override;

		bool onKeyDown(ui::Key) override;

		void onMouseOver() override;
		void onMouseOut() override;

	private:

		std::function<void()> m_callback;
		Text& m_label;
		Color m_normalColor;
        Color m_hoverColor;
        Color m_activeColor;
		State m_state;

		void fadeColor(Color from, Color to);

        void onRemove() override;
	};

} // namespace ui
