#include "DefineUtil.h"

util::Timer::Timer() {
	_time = 0;
	_start = std::chrono::high_resolution_clock::now();
}

ulint util::Timer::time() {
	return _time == 0
		   ? _time =
			   (std::chrono::high_resolution_clock::now() - _start).count()
		   : _time;
}
