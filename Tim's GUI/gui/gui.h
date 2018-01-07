#pragma once

#include "window.h"
#include "text.h"
#include "textentry.h"
#include "context.h"

namespace ui {

	Window* root();

	void addKeyboardCommand(sf::Keyboard::Key trigger_key, const std::function<void()>& handler);
	void addKeyboardCommand(sf::Keyboard::Key trigger_key, const std::vector<sf::Keyboard::Key>& required_keys, const std::function<void()>& handler);
	void setQuitHandler(const std::function<bool()>& handler);

	long double getProgramTime();

	vec2 getScreenSize();

	vec2 getMousePos();

	Context& getContext();

	void init(unsigned width = 500, unsigned height = 400, std::string title = "Behold", int target_fps = 30);

	void quit(bool force = false);

	void run();
}