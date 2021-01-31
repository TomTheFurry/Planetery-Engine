#include "Global.h"

float Global::windowRatio() {
	return windowSize.x / windowSize.y;
}

Global* global = new Global();