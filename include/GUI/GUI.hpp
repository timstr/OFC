#pragma once

#ifndef TIMSGUI_GUI_H
#define TIMSGUI_GUI_H

#include <SFML/Main.hpp>
#include "Element.hpp"
#include "Text.hpp"
#include "TextEntry.hpp"
#include "Context.hpp"
#include "Image.hpp"

namespace ui {

	// returns the global element to which other elements must be added to be used
	Element& root();

	// add a keyboard command that invokes `handler` when `trigger_key` is pressed
	void addKeyboardCommand(Key trigger_key, std::function<void()> handler);

	// add a keyboard command that invokes `handler` when `trigger_key` is pressed and `required_keys` are held
	void addKeyboardCommand(Key trigger_key, std::vector<Key> required_keys, std::function<void()> handler);

	// set a function to be invoked when the application is closed.
	// if the function returns false, the application will not close and
	// will resume unless a forced quit is happening
	void setQuitHandler(std::function<bool()> handler);

	// begin a transition
	// duration		- the length of the transition, in seconds
	// transitionFn	- the function to be invoked during every tick of the animation,
	//				  with an argument of 0 at the very beginning to 1 at the very end
	//				  The actual transition depends on what this function does. Typically,
	//				  it will use the argument to interpolate and assign a desired quantity
	// onComplete	- option function to be invoked when the transition is complete
	void startTransition(float duration, std::function<void(float)> transitionFn, std::function<void()> onComplete = {});

	// get the program time, in seconds
	float getProgramTime();

	// get the application's screen size
	vec2 getScreenSize();

	// get the current mouse position relative to the application's top left corner
	vec2 getMousePos();

	// returns true if the program window is currently in focus
	bool programInFocus();

	// create the application window
	void init(unsigned width = 500, unsigned height = 400, std::string title = "Behold", int target_fps = 30);

	// close the application
	void quit(bool force = false);

	// run the application
	void run();

} // namespace ui

#endif // TIMSGUI_GUI_H