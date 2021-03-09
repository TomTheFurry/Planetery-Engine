#pragma once

#include <array>

template<typename I, typename R, size_t N> class RollingAverage
{
  public:
	unsigned int index;
	I runningTotal;
	I buffer[N];

	RollingAverage() { clear(); }

	void clear() {
		runningTotal = 0;
		std::fill(buffer, buffer + N, 0);
		index = 0;
	}

	R next(I inputValue) {
		/*add new value*/
		runningTotal -= buffer[index];
		buffer[index] = inputValue;
		index++;
		index %= N;
		/*update running total*/
		runningTotal += inputValue;
		/*calculate average*/
		return static_cast<R>(runningTotal) / static_cast<R>(N);
	}

	R get() { return static_cast<R>(runningTotal) / static_cast<R>(N); }
};