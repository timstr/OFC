#pragma once

#include <GUI/Control.hpp>
#include <GUI/Container.hpp>
#include <GUI/BoxElement.hpp>

namespace ui {

    class CallbackButton : public Control, public Container, public BoxElement {
	private:
		enum class State {
			Normal,
			Hover,
			Active
		};

    public:
        CallbackButton(const String& label, const sf::Font& font, std::function<void()> on_click = {});

		void setNormalColor(Color);
		Color getNormalColor() const;
		void setHoverColor(Color);
		Color getHoverColor() const;
		void setActiveColor(Color);
		Color getActiveColor() const;

		void setCallback(std::function<void()> _callback);

		void setCaption(const String& _label);

		bool onLeftClick(int) override;
		void onLeftRelease() override;

		bool onKeyDown(ui::Key key) override;

		void onMouseOver() override;
		void onMouseOut() override;

        void render(sf::RenderWindow&) override;

	private:

		std::function<void()> callback;
		Text& label;
		Color normal_color, hover_color, active_color;
		State state;

		void fadeColor(Color from, Color to);
	};

} // namespace ui
