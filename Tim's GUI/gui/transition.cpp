#include "transition.h"
#include "gui.h"

namespace ui {

	Transition::Transition(Window* _target, double _duration, std::function<void(double)> _transitionFn, std::function<void()> _onComplete)
		: target(_target), duration(_duration), transitionFn(_transitionFn), onComplete(_onComplete), timestamp(getProgramTime()), completed(false) {

	}
	void Transition::apply(){
		double progress = (getProgramTime() - timestamp) / duration;
		if (progress > 1){
			transitionFn(1.0);
			if (onComplete){
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