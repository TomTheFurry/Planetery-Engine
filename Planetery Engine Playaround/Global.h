#pragma once
#include <glm/glm.hpp>
#include "Manager.h"

using vec2 = glm::vec2;

class Global {
public:
	vec2 windowSize = vec2(1920,1080);
	float fov = 60.0f;
	float windowRatio();
	int targetTps = 60;
	unsigned int galaxySeed = 0;

	//Managers
	WorldManager* worldManager;
	AnchorManager* anchorManager;
	PlayerManager* playerManager;

};

extern Global* global;

