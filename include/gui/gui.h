#pragma once

#include <SFML/Main.hpp>
#include "element.h"
#include "text.h"
#include "textentry.h"
#include "context.h"

namespace ui {

	Element& root();
	
	void addKeyboardCommand(Key trigger_key, std::function<void()> handler);
	void addKeyboardCommand(Key trigger_key, std::vector<Key> required_keys, std::function<void()> handler);
	void setQuitHandler(std::function<bool()> handler);

	void startTransition(float duration, std::function<void(float)> transitionFn, std::function<void()> onComplete = {});

	double getProgramTime();

	vec2 getScreenSize();

	vec2 getMousePos();

	Context& getContext();

	void init(unsigned width = 500, unsigned height = 400, std::string title = "Behold", int target_fps = 30);

	void quit(bool force = false);

	void run();
}