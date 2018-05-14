#pragma once

#include "window.h"

namespace ui {

	// TODO: remove 'target'
	struct Transition {
		Transition(double _duration, std::function<void(double)> _transitionFn, std::function<void()> _onComplete = {});

		void apply();

		bool complete() const;

		private:
		std::function<void(double)> transitionFn;
		std::function<void()> onComplete;
		bool completed;
		double duration;
		long double timestamp;
	};

}