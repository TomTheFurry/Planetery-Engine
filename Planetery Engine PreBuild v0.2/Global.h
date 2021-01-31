#pragma once

#include <glm/glm.hpp>
#include "utility.h"
#include "Manager.h"

class Global {
public:
	Global();
	vec2 windowSize = vec2(1920,1080);
	float fov = 60.0f;
	float nearView = 0.1f;
	float farView = 1000.0f;
	float windowRatio();
	int targetTps = 60;
	uint fileSeed = 0;
	uint playerLevel = 0;
	Anchor* main;

	//Managers
	MapFileManager* mapFileManager;
	WorldManager* worldManager;
	RenderManager* renderManager;

	//World Gen RNG
	SeededRng rng = SeededRng(0);

};

extern Global* global;


