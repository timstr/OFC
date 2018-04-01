#pragma once

#include "window.h"

namespace ui {

	struct Transition {
		Transition(Window* _target, double _duration, std::function<void(double)> _transitionFn, std::function<void()> _onComplete = {});

		void apply();

		bool complete() const;

		const Window* target;

		private:
		std::function<void(double)> transitionFn;
		std::function<void()> onComplete;
		bool completed;
		double duration;
		long double timestamp;
	};

}