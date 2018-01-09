#pragma once

#include <SFML\Graphics.hpp>
#include <vector>
#include <functional>
#include <set>

typedef sf::Vector2f vec2;


namespace ui {

	// TODO: automatic sizing based on child windows
	struct Window {
		bool disabled = false;
		bool visible = true;
		bool clipping = false;
		bool bring_to_front = false;
		vec2 pos;
		vec2 size;

		virtual ~Window();

		void close();

		virtual bool hit(vec2 testpos);

		vec2 localMousePos();
		vec2 absPos();

		virtual void onLeftClick(int clicks);
		virtual void onLeftRelease();
		virtual void onRightClick(int clicks);
		virtual void onRightRelease();
		bool leftMouseDown();
		bool rightMouseDown();

		virtual void onScroll(double delta_x, double delta_y);

		void startDrag();
		// TODO: add explicit stopDrag instead of automatically stopping when mouse button lifted?
		virtual void onDrag();

		virtual void onHover();
		virtual void onHoverWithWindow(Window *drag_window);

		// shall return false if the parent's method is to be invoked
		virtual bool onDropWindow(Window *window);

		virtual void onFocus();
		bool isFocused();
		virtual void onLoseFocus();
		void grabFocus();
		void focusToNextWindow();
		void focusToPreviousWindow();

		virtual void onKeyDown(sf::Keyboard::Key key);
		virtual void onKeyUp(sf::Keyboard::Key key);
		bool keyDown(sf::Keyboard::Key key);

		struct Alignment {
			enum Type {
				None,
				After,
				Before,
				Center,
				InsideMin,
				InsideMax
			};

			protected:
			float margin;

			Alignment(Type _type, Window* _relative_to = nullptr, float _margin = 0.0f);

			Type type;
			Window* relative_to;

			friend struct Window;
		};

		struct XAlignment : Alignment {
			private:

			XAlignment(Type type, Window* relative_to = nullptr, float margin = 0.0f);

			friend struct Window;
		};
		struct YAlignment : Alignment {
			private:

			YAlignment(Type type, Window* relative_to = nullptr, float margin = 0.0f);

			friend struct Window;
		};

		void setXAlign(XAlignment xalignment = XAlignment(Alignment::None, nullptr));
		void setYAlign(YAlignment yalignment = YAlignment(Alignment::None, nullptr));
		void align();
		void alignChildren();
		void alignAndAutoSize(float margin);

		static XAlignment noAlignX();
		static XAlignment leftOf(Window* window, float margin = 0.0f);
		static XAlignment rightOf(Window* window, float margin = 0.0f);
		static XAlignment middleOfX(Window* window);
		static XAlignment insideLeft(Window* window, float margin = 0.0f);
		static XAlignment insideRight(Window* window, float margin = 0.0f);
		static YAlignment noAlignY();
		static YAlignment above(Window* window, float margin = 0.0f);
		static YAlignment below(Window* window, float margin = 0.0f);
		static YAlignment insideTop(Window* window, float margin = 0.0f);
		static YAlignment insideBottom(Window* window, float margin = 0.0f);
		static YAlignment middleOfY(Window* window);

		void addChildWindow(Window* window);
		void addChildWindow(Window* window, vec2 pos);
		void addChildWindow(Window* window, XAlignment xalignment);
		void addChildWindow(Window* window, YAlignment yalignment);
		void addChildWindow(Window* window, XAlignment xalignment, YAlignment yalignment);
		void addChildWindow(Window* window, float xpos, YAlignment yalignment);
		void addChildWindow(Window* window, XAlignment xalignment, float ypos);
		void releaseChildWindow(Window* window);
		void bringToFront();
		void clear();

		Window* findWindowAt(vec2 _pos);

		virtual void render(sf::RenderWindow& renderwindow);
		void renderChildWindows(sf::RenderWindow& renderwindow);

		void startTransition(double duration, const std::function<void(double)> transitionFn, const std::function<void()>& onComplete = {});

		const std::vector<Window*>& getChildWindows() const;
		Window* getParent() const;

		private:
		Window* parent = nullptr;
		std::vector<Window*> childwindows;

		XAlignment xalign = XAlignment(Alignment::None, nullptr);
		YAlignment yalign = YAlignment(Alignment::None, nullptr);
		bool children_aligned = false;

		void alignWindows(const std::vector<Window*>& windows);
		vec2 getBounds(const std::vector<Window*>& windows);

		friend struct Context;
	};

} // namespace ui