#include "gui/transition.h"
#include "gui/gui.h"

namespace ui {

	Transition::Transition(float _duration, std::function<void(float)> _transitionFn, std::function<void()> _onComplete)
		: duration(_duration), transitionFn(_transitionFn), onComplete(_onComplete), timestamp(getProgramTime()), completed(false) {

	}

	void Transition::apply() {
		float progress = (float)(getProgramTime() - timestamp) / duration;
		if (progress >= 1.0) {
			transitionFn(1.0);
			if (onComplete) {
				onComplete();
			}
			completed = true;
		} else {
			transitionFn(progress);
		}
	}

	bool Transition::complete() const {
		return completed;
	}

}