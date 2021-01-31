#include "RollingAverage.h"



double testerFunction(double var) {
	RollingAverage<double, double, 0> roller{};
	for (unsigned int i = 0; i < 1000; i++) {
		roller.next(var);
		var += i * var;
	}
	return roller.get();
}
