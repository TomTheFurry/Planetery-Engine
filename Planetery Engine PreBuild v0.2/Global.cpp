#include "Global.h"

Global::Global()
{
}

float Global::windowRatio() {
	return windowSize.x / windowSize.y;
}

Global* global = new Global();