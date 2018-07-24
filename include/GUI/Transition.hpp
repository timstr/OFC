#pragma once

#include "Element.hpp"

namespace ui {

	struct Transition {
		Transition(float _duration, std::function<void(float)> _transitionFn, std::function<void()> _onComplete = {});

		void apply();

		bool complete() const;

	private:
		std::function<void(float)> transitionFn;
		std::function<void()> onComplete;
		bool completed;
		float duration;
		float timestamp;
	};

}